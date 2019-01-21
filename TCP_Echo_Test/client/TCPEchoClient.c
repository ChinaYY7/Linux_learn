#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPClientUtility.h"

int main(int argc, char *argv[])
{
    if(argc < 2 || argc > 3)
        Deal_User_Error("wrong arguments","<Server Address> <Server Port>");
    
    char *server = argv[1];
    char *service = (argc == 3) ? argv[2] : "echo";

    int sock = SetupTCPClientSocket(server, service);

    if (sock < 0)
        Deal_User_Error("SetupTCPClientSocket() faild!", "Unable to connect");

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