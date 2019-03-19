#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "Ftp_ClientUtility.h"
#include "Trans_Protocol.h"
#include "Encode.h"
#include "Framer.h"


#define SERVER_LEN 100
#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

int main(int argc, char *argv[])
{
    FILE *Config_file_fp;
    char addrBuffer[SERVER_LEN];
    char server[SERVER_LEN], service[SERVICE_LEN];
    char *token;

    if(argc ==3)
    {
        strcpy(server, argv[1]);
        strcpy(service, argv[2]);
    }
    else if(argc == 1)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);
    }
    else
        Deal_User_Error("wrong arguments","<Server Address> <Server Port>",ERROR_VALUE);
    
    int sock = SetupTCPClientSocket(server, service);

    //if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)//设置非阻塞
        //Deal_System_Error("Unable to put client sock into non-blocking/async mode", ERROR_VALUE);

    if (sock < 0)
        Deal_User_Error("SetupTCPClientSocket() faild!", "Unable to connect",ERROR_VALUE);

    FILE *str = fdopen(sock, "r+");
    if(str == NULL)
        Deal_System_Error("fdopen() faild", ERROR_VALUE);

    char echoString[BUFFSIZE];
    size_t echoStringLen;
    ssize_t numBytes;
    char buffer[BUFFSIZE];

    TransInfo vi;
    uint8_t inbuf[MAX_WIRE_SIZE];
    ssize_t Get_size;
    ssize_t write_count;

    memset(&vi, 0, sizeof(vi));

    FILE *tmp_fp;
    if((tmp_fp = fopen(TMP_Path,"w")) == NULL)
        Deal_User_Error("fopen() error", "create temp file failed",ERROR_VALUE);    

    while(1)
    {
        printf("\nPlease input sended string:   ");
        fgets(echoString, BUFFSIZE, stdin);

        echoStringLen = strlen(echoString);

        numBytes = TCP_nSend(sock, echoString, echoStringLen);
        if(numBytes < 0)
            exit(ERROR_VALUE);
        printf("send %lu bytes\n", numBytes);

        while(1)
        {
            Get_size = GetNextMsg(str,inbuf,MAX_WIRE_SIZE);
            Decode(&vi, inbuf, MAX_WIRE_SIZE);
            fseek(tmp_fp, vi.offset, SEEK_SET);
            printf("header=%x, offset=%d, date_size=%d\n",vi.header,vi.offset,vi.date_size);
            write_count = fwrite(vi.data,sizeof(char),vi.date_size,tmp_fp);
            fflush(tmp_fp);
            printf("writecount=%d\n",write_count);
            if(vi.date_size < BUFFER_SIZE)
            {
                system("cat temp");
                break;
            }
        }

        //printf("Received from sever %lu bytes", numBytes);
    }

    close(sock);
    return 0;
} 