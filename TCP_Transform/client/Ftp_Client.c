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
            User_Error_Exit("fopen()", "connect.config not exit!");

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);
    }
    else
        User_Error_Exit("wrong arguments","<Server Address> <Server Port>");
    
    int sock = SetupTCPClientSocket(server, service);

    //if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)//设置非阻塞
        //System_Error_Exit("Unable to put client sock into non-blocking/async mode");

    if (sock < 0)
        User_Error_Exit("SetupTCPClientSocket() faild!", "Unable to connect");

    FILE *str = fdopen(sock, "r+");
    if(str == NULL)
        System_Error_Exit("fdopen() faild");

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

    while(1)
    {
        printf("\nftp@cmd:   ");
        fgets(echoString, BUFFSIZE, stdin);

        echoStringLen = strlen(echoString);

        numBytes = TCP_nSend(sock, echoString, echoStringLen);
        if(numBytes < 0)
            exit(ERROR_VALUE);

        if((tmp_fp = fopen(TMP_Path,"w+")) == NULL)
            User_Error_Exit("fopen() error", "create temp file failed");    

        while(1)
        {
            Get_size = GetNextMsg(str,inbuf,MAX_WIRE_SIZE);
            Decode(&vi, inbuf, MAX_WIRE_SIZE);
            fseek(tmp_fp, vi.offset, SEEK_SET);
            //printf("header=%x, offset=%d, date_size=%d\n",vi.header,vi.offset,vi.date_size);
            write_count = fwrite(vi.data,sizeof(char),vi.date_size,tmp_fp);
            
            //printf("writecount=%d\n",write_count);
            if(vi.date_size < BUFFER_SIZE)
            {
                fclose(tmp_fp);
                system("cat temp");
                break;
            }
        }
    }

    close(sock);
    return 0;
} 