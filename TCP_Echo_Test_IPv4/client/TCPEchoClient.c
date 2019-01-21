#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"

int main(int argc, char *argv[])
{
    if(argc < 2 || argc > 3)
        Deal_User_Error("wrong arguments","<Server Address> <Server Port>");
    
    char *servIP = argv[1];

    in_port_t servPort = (argc == 3) ? atoi(argv[2]) : 7;//7 is well-known echo port

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0)
        Deal_System_Error("socket() faild!");

    struct sockaddr_in servAddr;

    memset(&servAddr, 0 , sizeof(servAddr));
    servAddr.sin_family = AF_INET;

    int rtnval = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
    //printf("servAddr.sin_addr.s_addr = %x\n",servAddr.sin_addr.s_addr);

    if(rtnval <= 0)
        Deal_User_Error("inet_pton faild","invalid address string");

    servAddr.sin_port = htons(servPort);

    if(connect(sock, (struct sockaddr *)&servAddr,sizeof(servAddr)) < 0)
        Deal_System_Error("connect faild!");

    char echoString[BUFFIZE];
    size_t echoStringLen;
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;
    char buffer[BUFFIZE];

    while(1)
    {
        printf("\nPlease input sended string:");
        fgets(echoString, BUFFIZE, stdin);

        echoStringLen = strlen(echoString);
        numBytes = send(sock, echoString, echoStringLen, 0);
        if(numBytes < 0)
            Deal_System_Error("send faild");
        else if (numBytes != echoStringLen)
            Deal_User_Error("send","sent unexpected number of bytes");
        printf("%lu bytes have been sent\n", numBytes);

        printf("Received from sever:");
        totalBytesRcvd = 0;
        while (totalBytesRcvd < echoStringLen)
        {
            numBytes = recv(sock, buffer, BUFFIZE - 1, 0);
            if(numBytes < 0)
                Deal_System_Error("recv() faild\n");
            else if(numBytes == 0)
                Deal_User_Error("recv", "connection closed prematurely");
            totalBytesRcvd += numBytes;
            buffer[numBytes] = '\0';
            fputs(buffer,stdout);
        }
        printf("%u bytes have been received\n", totalBytesRcvd);
        fputc('\n',stdout);
    }
    close(sock);
    return 0;
} 