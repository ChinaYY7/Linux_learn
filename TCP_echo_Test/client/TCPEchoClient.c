#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
int main(int argc, char *argv[])
{
    if(argc < 3 || argc > 4)
        Deal_User_Error("wrong arguments","<Server Address> <Echo Word> <Server Port>");
    
    char *servIP = argv[1];
    char *echoString = argv[2];

    in_port_t servPort = (argc == 4) ? atoi(argv[3]) : 7;//7 is well-known echo port

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock < 0)
        Deal_System_Error("socket() faild!");

    struct sockaddr_in servAddr;

    memset(&servAddr, 0 , sizeof(servAddr));
    servAddr.sin_family = AF_INET;

    int rtnval = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);

    if(rtnval <= 0)
        Deal_User_Error("inet_pton faild","invalid address string");

    servAddr.sin_port = htons(servPort);

    if(connect(sock, (struct sockaddr *)&servAddr,sizeof(servAddr)) < 0)
        Deal_System_Error("connect faild!");

    size_t echoStringLen = strlen(echoString);
    ssize_t numBytes = send(sock, echoString, echoStringLen, 0);

    if(numBytes < 0)
        Deal_System_Error("send faild");
    else if (numBytes != echoStringLen)
        Deal_User_Error("send","sent unexpected number of bytes");

    unsigned int totalBytesRcvd = 0;
    printf("Rceived:");

    while (totalBytesRcvd < echoStringLen)
    {
        char buffer[4096];

        numBytes = recv(sock, buffer, 4095, 0);
        if(numBytes < 0)
            Deal_System_Error("recv() faild\n");
        else if(numBytes == 0)
            Deal_User_Error("recv", "connection closed prematurely");
        totalBytesRcvd += numBytes;
        buffer[numBytes] = '\0';
        //printf("recive: %s\n", buffer);
        fputs(buffer,stdout);
    }
    fputc('\n',stdout);
    
    close(sock);
    return 0;
} 