#include "Trans_Protocol.h"
#include "Encode.h"
#include "Framer.h"
#include "Deal_Error.h"
int Trans_Send(FILE *out, TransInfo *v, uint16_t head)
{
    uint8_t buf[MAX_WIRE_SIZE];

    Encode(v, buf, MAX_WIRE_SIZE, head);  //编码
    int Send_size = PutMsg(buf,sizeof(TransInfo),out);//成帧发送

    return Send_size;
}

int Trans_Recv(FILE *in, TransInfo *v)
{
    uint8_t buf[MAX_WIRE_SIZE];
    //读取接收缓冲区数据
    int Recv_size = GetNextMsg(in,buf,MAX_WIRE_SIZE);
    if(Recv_size < 0)
        User_Error_Exit("GetNextMsg error", "Date_size larger than bufsize");

     //数据解码
    Bool Decode_sta = Decode(v, buf, MAX_WIRE_SIZE);
    if(!Decode_sta)
        User_Error_Exit("Decode error", "Head is not matching");

    return Recv_size;
}

int Send_Messege(FILE *out, const char *string)
{
    TransInfo vi;
    memset(&vi, 0, sizeof(vi));

    strcpy(vi.data, string);
    vi.date_size = strlen(vi.data) + 1;  //加上字符串结束符
    int Send_size = Trans_Send(out, &vi, MESSEGE_HEAD);

    return Send_size;
}

int Recv_Messege(FILE *in, char *string)
{
    TransInfo vi;
    memset(&vi, 0, sizeof(vi));

    int Recv_size = Trans_Recv(in, &vi);
    if(vi.header == MESSEGE_HEAD)
        strcpy(string, vi.data);
    else
        return -1;
    
    return Recv_size;
}

uint64_t Send_File(FILE *out, const char *file_path)
{
    FILE *file_fp;
    TransInfo vi;
    uint64_t Send_File_Bytes = 0;

    if((file_fp = fopen(file_path,"r")) == NULL)
        User_Error_Exit("fopen() error", "file not exit");
    
    while(1)
    {
        vi.offset = ftell(file_fp);  //获取当前文件偏移量
        uint16_t ReadCount = fread(vi.data,sizeof(uint8_t),BUFFER_SIZE,file_fp);

        //处理读到的最后一块，一般最后一块小于一整块的数量（一块4096字节）
        if(ReadCount < BUFFER_SIZE)
        {
            if(ferror(file_fp))
                User_Error_Exit("fread()","error");
        }
        vi.date_size = ReadCount;
        Send_File_Bytes += Trans_Send(out, &vi, FILE_HEAD);

        //读到文件尾
        if(feof(file_fp))
            break;
    }
    fclose(file_fp);
    return Send_File_Bytes;
}

uint64_t Recv_File(FILE *in, const char *file_path)
{
    FILE *file_fp;
    TransInfo vi;
    uint64_t Recv_File_Bytes = 0;

    memset(&vi, 0, sizeof(vi));
    //以截断、读写、不存在则创建、方式打开文件，该文件用以接收服务端返回的指令执行结果
    if((file_fp = fopen(file_path,"w+")) == NULL)
        User_Error_Exit("fopen() error", "create file failed");

    while(1)
    {
        Recv_File_Bytes += Trans_Recv(in, &vi);

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
            return -1;
    }
    fclose(file_fp);
    return Recv_File_Bytes;
}