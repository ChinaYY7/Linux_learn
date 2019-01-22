#include "apue.h"
#include <sys/socket.h>
#include "Deal_Error.h"
#include "UDPClientUtility.h"

int main(int argc, char *argv[])
{
    if(argc < 2 || argc > 3)
        Deal_User_Error("wrong arguments","<Server Address> <Server Port>");
    
    char *server = argv[1];
    char *service = (argc == 3) ? argv[2] : "echo";
    struct addrinfo *severAddr;

    int sock = SetupUDPClientSocket(server, service, &severAddr);

    if (sock < 0)
        Deal_User_Error("SetupUDPClientSocket() faild!", "Unable to connect");

    char echoString[MAXSTRLEN * 2];
    size_t echoStringLen;
    ssize_t numBytes;
    char buffer[MAXSTRLEN + 1];
    struct sockaddr_storage fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    while(1)
    {
        printf("\nPlease input sended string:   ");
        fgets(echoString, BUFFIZE, stdin);

        echoStringLen = strlen(echoString);
        if(echoStringLen > MAXSTRLEN)
            Deal_User_Error("echoString","String too long!");

        numBytes = sendto(sock, echoString, echoStringLen, 0, severAddr->ai_addr, severAddr->ai_addrlen);
        if(numBytes < 0)
            Deal_System_Error("sendto() faild");
        else if (numBytes != echoStringLen)
            Deal_User_Error("sendto() error","sent unexpected number of bytes");
        printf("send %lu bytes\n", numBytes);
        
        numBytes = recvfrom(sock, buffer, MAXSTRLEN, 0,(struct sockaddr *)&fromAddr, &fromAddrLen);
        if(numBytes < 0)
            Deal_System_Error("recvfrom() faild\n");
        else if(numBytes != echoStringLen)
            Deal_User_Error("recvfrom() error", "recevied unexpected number of bytes");

        if(!SockAddrsEqual(severAddr->ai_addr, (struct sockaddr *)&fromAddr))
            Deal_User_Error("recvfrom()", "recevied a packet from unknown source!");

        freeaddrinfo(severAddr);

        buffer[echoStringLen] = '\0';
        printf("Received from sever(%lu bytes): %s\n", numBytes, buffer);
    }
    
    close(sock);
    return 0;
} 