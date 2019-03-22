
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "Deal_Error.h"
#include "Ftp_ClientUtility.h"
#include "Trans_Protocol.h"
#include "Encode.h"
#include "Framer.h"

int main(int argc, char *argv[])
{
    char server[SERVER_LEN], service[SERVICE_LEN];
    
    //从命令行或者配置文件获取服务器地址和端口号
    Get_Server_Config(server, service, argc, argv);
    
    //创建客服端命令端连接套接字
    int sock = SetupTCPClientSocket(server, service);
    if (sock < 0)
        User_Error_Exit("SetupTCPClientSocket() faild!", "Unable to connect");

    //使用流包装套接字
    FILE *str = fdopen(sock, "r+");
    if(str == NULL)
        System_Error_Exit("fdopen() faild");

    char cmd[BUFFSIZE];
    uint8_t inbuf[MAX_WIRE_SIZE];

    TransInfo vi;
    memset(&vi, 0, sizeof(vi));

    FILE *tmp_fp;

    while(1)
    {
        //发送命令
        printf("\nftp@cmd:   ");
        fgets(cmd, BUFFSIZE, stdin);
        size_t cmdStringLen = strlen(cmd);
        ssize_t numBytes = TCP_nSend(sock, cmd, cmdStringLen);

        //以截断、读写、不存在则创建、方式打开文件，该文件用以接收服务端返回的指令执行结果
        if((tmp_fp = fopen(TMP_Path,"w+")) == NULL)
            User_Error_Exit("fopen() error", "create temp file failed");    

        while(1)
        {
            //读取接收缓冲区数据
            int Get_size = GetNextMsg(str,inbuf,MAX_WIRE_SIZE);
            if(Get_size <= 0)
                User_Error_Exit("GetNextMsg error", "Date is wrong");

            //数据解码
            Bool Decode_sta = Decode(&vi, inbuf, MAX_WIRE_SIZE);
            if(!Decode_sta)
                User_Error_Exit("Decode error", "Head is not matching");

            //根据接收的数据信息设置该数据的所在文件的偏移量，并写入文件
            fseek(tmp_fp, vi.offset, SEEK_SET);
            ssize_t write_count = fwrite(vi.data,sizeof(char),vi.date_size,tmp_fp);
            
            //服务端发送的数据已接收写入完成
            if(vi.date_size < BUFFER_SIZE)
            {
                fclose(tmp_fp);
                system("cat ./temp/temp");
                break;
            }
        }
    }

    close(sock);
    return 0;
} 