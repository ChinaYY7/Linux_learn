#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPClientUtility.h"

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
            Deal_User_Error("fopen()", "connect.config not exit!");

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);
    }
    else
        Deal_User_Error("wrong arguments","<Server Address> <Server Port>");
    
    int sock = SetupTCPClientSocket(server, service);

    if (sock < 0)
        Deal_User_Error("SetupTCPClientSocket() faild!", "Unable to connect");

    char echoString[BUFFIZE];
    size_t echoStringLen;
    ssize_t numBytes;
    char buffer[BUFFIZE];

    while(1)
    {
        printf("\nPlease input sended string:   ");
        fgets(echoString, BUFFIZE, stdin);

        echoStringLen = strlen(echoString);

        numBytes = TCP_nSend(sock, echoString, echoStringLen);
        if(numBytes < 0)
            exit(Exit_value);
        printf("send %lu bytes\n", numBytes);

        numBytes = TCP_nReceive(sock, buffer, echoStringLen);
        buffer[echoStringLen] = '\0';
        printf("Received from sever(%lu bytes): %s\n", numBytes, buffer);
    }

    close(sock);
    return 0;
} 