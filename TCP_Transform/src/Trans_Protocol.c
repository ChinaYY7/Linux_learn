#include <netinet/in.h>
#include "Trans_Protocol.h"
#include "Deal_Error.h"
#include "TCP_Socket.h"

size_t Compute_TransInfo_Size(TransInfo *v)
{
    return sizeof(v->header) + sizeof(v->date_size) + sizeof(v->offset) + v->date_size;
}
//编码
size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize, uint16_t head)
{

    if(bufsize < sizeof(TransInfo))
        User_Error_Exit("Output buffer too small","");

    TransInfo *vm = (TransInfo *)outbuf;
    memset(outbuf, 0, sizeof(TransInfo));
    v->header= head;

    //printf("header=%x, offset=%d, date_size=%d\n",v->header,v->offset,v->date_size);

    if(head == FILE_HEAD)  //编码的是文件内容
        vm->offset = htonl(v->offset);

    vm->header = htons(v->header);
    vm->date_size = htons(v->date_size);

    for(int i = 0; i < v->date_size; i++)
        vm->data[i] = v->data[i];

    return sizeof(TransInfo);
}

//解码
Bool Decode(TransInfo *v, uint8_t *inbuf, const size_t msize)
{

    TransInfo *vm = (TransInfo *)inbuf;
    
    v->header = ntohs(vm->header);
    
    if(msize < sizeof(TransInfo))
        User_Error_Exit("INBUF","too small !");
    

    if(v->header == FILE_HEAD)    //解码文件
    {
        v->offset =  ntohl(vm->offset);
        v->date_size = ntohs(vm->date_size);
        for(int i = 0; i < v->date_size; i++)
            v->data[i] = vm->data[i];

        return True;
    }
    else if(v->header == MESSEGE_HEAD) //解码消息
    {
        v->date_size = ntohs(vm->date_size);
        for(int i = 0; i < v->date_size; i++)
            v->data[i] = vm->data[i];

        return True;
    }
    else
        return False;
}

//读取一帧数据异常判断
int GetNextMsg_Return_Value(FILE *in, const char *str)
{
    if(feof(in)) 
    {
        printf("read %s disconnect\n", str);
        System_Error_Exit("fread()");
        return FREAD_EOF;
    }    
    else if(ferror(in))
    {
        //printf("read %s error\n", str);
        //System_Error("fread(%s)", str);
        return FREAD_ERROR;
    }
    else
    {
        return FREAD_ELSE;
    }
}
//读取一帧信息
int GetNextMsg(FILE *in, uint8_t *buf, size_t buffsize)
{
    uint16_t msize = 0;
    uint16_t extra = 0;
    uint16_t fread_count;
    static uint8_t read_msize_sta = 1, read_buf_sta = 0;
    static int read_count = 0; 
    
    int rnt;  
    
    if(read_msize_sta)
    {
        //printf("\nread msize\n");
        rnt = fread(&msize, sizeof(uint16_t), 1, in);
        read_count++;
        msize = ntohs(msize);
        //printf("rnt = %d, msize(%d) = %d\n",rnt, read_count,msize);
        if(rnt != 1)
        {
            //printf("fread misze rnt = %d, errno = %d ", rnt, errno);
            return GetNextMsg_Return_Value(in, "msize");
        }

        read_msize_sta = 0;
        read_buf_sta = 1;

        if(msize > buffsize)
        {
            printf("msize(%d) > buffsize(%ld)", msize, buffsize);
            extra = msize - buffsize;
            msize = buffsize;
        }
    }
    if(read_buf_sta)
    {
       // printf("read buf\n");
        rnt = fread(buf, sizeof(uint8_t), msize, in);
        //printf("rnt = %d\n",rnt);
        if(rnt < msize)
        {
            while(GetNextMsg_Return_Value(in, "buf") == FREAD_ERROR)
            {
                buf+=rnt;
                msize = msize - rnt;
                rnt = fread(buf, sizeof(uint8_t), msize, in);
                //printf("rnt = %d\n",rnt);
                if(rnt == msize)
                    break;
            }
            //printf("fread buf rnt = %d, errno = %d ", rnt, errno);
        }

        read_buf_sta = 0;
        read_msize_sta = 1;
        
        if(extra > 0)
        {
            uint8_t waste[BUFFSIZE];
            rnt = fread(buf, sizeof(uint8_t), extra, in);
            if(rnt != extra)
            {
                return GetNextMsg_Return_Value(in, "waste");
            }
            return -(msize + extra + sizeof(uint16_t));
        }
        else
            return rnt + sizeof(uint16_t);
    }
}

//写一帧信息
//若发送缓冲区有空间且可用返回写入的字节数
//若缓存区空间不足或者数据暂时不可用，阻塞，直到有空间可用
//若连接断开，将报错，并返回0
int PutMsg(const uint8_t *buf, size_t msgsize, FILE *out)
{
    if(msgsize > __UINT16_MAX__)
    {
        User_Error("bufsize too long", "should < __UINT16_MAX__");
        return BUFSIZE_TOO_LONG;
    }
    
    uint16_t payloadsize = htons(msgsize);
    
    if((fwrite(&payloadsize, sizeof(uint16_t), 1, out) != 1) || (fwrite(buf, sizeof(uint8_t), msgsize, out) != msgsize))
    {
        System_Error("fwrite(payloadsize or buf)");
        return FWRITE_ERROR;
    }
    
    fflush(out);

    return msgsize + sizeof(payloadsize);
}

//编码发送
int Trans_Send(FILE *out, TransInfo *v, uint16_t head)
{
    uint8_t buf[MAX_WIRE_SIZE];

    Encode(v, buf, MAX_WIRE_SIZE, head);  //编码
    int Send_size = PutMsg(buf,Compute_TransInfo_Size(v),out);//成帧发送

    return Send_size;
}

//接收解码
int Trans_Recv(FILE *in, TransInfo *v)
{
    uint8_t buf[MAX_WIRE_SIZE];

    //读取接收缓冲区数据
    int Recv_size = GetNextMsg(in,buf,MAX_WIRE_SIZE);
    if(Recv_size < 1)
        return Recv_size;
        
     //数据解码
    Bool Decode_sta = Decode(v, buf, MAX_WIRE_SIZE);
    if(!Decode_sta)
    {
        User_Error("Decode error", "Head is not matching");
        return DECODE_MATCHING_ERROR;
    }
    
    return Recv_size;
}

//发送消息
int Send_Messege(FILE *out, const char *string)
{
    TransInfo vi;
    memset(&vi, 0, sizeof(vi));

    strcpy(vi.data, string);
    vi.date_size = strlen(vi.data) + 1;  //加上字符串结束符
    int Send_size = Trans_Send(out, &vi, MESSEGE_HEAD);

    return Send_size;
}


//接收消息
//阻塞模式，接收到信息，返回接收的字节数，没有数据阻塞
//连接断开返回0，不是消息返回-1
int Recv_Messege(FILE *in, char *string)
{
    TransInfo vi;
    memset(&vi, 0, sizeof(vi));

    int Recv_size = Trans_Recv(in, &vi);
    if(Recv_size < 1)
        return Recv_size;
    if(vi.header == MESSEGE_HEAD)
        strcpy(string, vi.data);
    else
        return NOT_MATCHING;
    
    return Recv_size;
}

//发送文件
//文件不存在返回-1
//文件存在且成功发送 返回发送的大小
uint64_t Send_File(FILE *out, const char *file_path)
{
    FILE *file_fp;
    TransInfo vi;
    uint64_t Send_File_Bytes = 0;

    if((file_fp = fopen(file_path,"r")) == NULL)
    {
        User_Error("fopen() error", "file not exit");
        return FOPEN_ERROR;
    }
        
    while(1)
    {
        vi.offset = ftell(file_fp);  //获取当前文件偏移量
        uint16_t ReadCount = fread(vi.data,sizeof(uint8_t),BUFFER_SIZE,file_fp);

        //处理读到的最后一块，一般最后一块小于一整块的数量（一块4096字节）
        if(ReadCount < BUFFER_SIZE)
        {
            if(ferror(file_fp))
            {
                User_Error("fread(file_fp)","error");
                return FREAD_ERROR;
            }
        }
        vi.date_size = ReadCount;
        Send_File_Bytes += Trans_Send(out, &vi, FILE_HEAD);
        //printf("send %ld Bytes\n", Send_File_Bytes);
        //读到文件尾
        if(feof(file_fp))
            break;
    }
    fclose(file_fp);
    return Send_File_Bytes;
}

//接收文件
//文件创建失败返回-1
//文件创建成功，返回接收的数据大小
uint64_t Recv_File(FILE *in, const char *file_path)
{
    FILE *file_fp;
    TransInfo vi;
    uint64_t Recv_File_Bytes = 0;
    int Recv_Bytes = 0;
    static uint8_t File_Creat_Sta = 1;

    memset(&vi, 0, sizeof(vi));

    if(File_Creat_Sta)
    {
        //以截断、读写、不存在则创建、方式打开文件，该文件用以接收服务端返回的指令执行结果
        if((file_fp = fopen(file_path,"w+")) == NULL)
        {
            User_Error("fopen() error", "create file failed");
            return FOPEN_ERROR;
        }
        printf("Create file.....\n");
        File_Creat_Sta = 0;
    }
    else
    {
        if((file_fp = fopen(file_path,"r+")) == NULL)
        {
            User_Error("fopen() error", "Open file failed");
            return FOPEN_ERROR;
        }
    }
    
    while(1)
    {
        Recv_Bytes = Trans_Recv(in, &vi);
        if(Recv_Bytes < 1)
        {
            fclose(file_fp);
            return Recv_Bytes;
        }

        Recv_File_Bytes+=Recv_Bytes;

        if(vi.header == FILE_HEAD)
        {
            //根据接收的数据信息设置该数据的所在文件的偏移量，并写入文件
            fseek(file_fp, vi.offset, SEEK_SET);
            ssize_t write_count = fwrite(vi.data,sizeof(char),vi.date_size,file_fp);
                    
            //服务端发送的数据已接收写入完成
            if(vi.date_size < BUFFER_SIZE)
                break;
        }
        else
            return NOT_MATCHING;
    }
    fclose(file_fp);
    printf("finlish file.....\n");
    File_Creat_Sta = 1;
    return Recv_File_Bytes;
}

//接收数据，自行决定是消息还是文件
//uint64_t Recv_File_Ex()

Bool Deal_Cmd(char *Cmd_Buffer, char Parameter[][100], int Parameter_Num)
{
    char *token;
    int i;

    token = strtok(Cmd_Buffer, DELIM);
    strcpy(Parameter[0],token);

    for (i = 1; i < Parameter_Num; i++)
    {
        token = strtok(NULL, DELIM);
        if(token == NULL )
            break;
        strcpy(Parameter[i],token);
    }
    if(i < Parameter_Num)
        return False;
    return True;
}