#include "TCPServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h"
void HandleTCPClient(int clntSocket)
{
    char buffer[BUFFIZE];
    ssize_t numBytesRcved = 1;
    ssize_t numBytesSent;
    
    while(numBytesRcved > 0)
    {
        numBytesRcved = recv(clntSocket, buffer, BUFFIZE - 1, 0);
        if(numBytesRcved < 0)
            Deal_System_Error("recv faild!");
        printf("%ld bytes have been received\n", numBytesRcved);
        buffer[numBytesRcved] = '\0';
        printf("Recived from client: %s\n",buffer);

        numBytesSent = send(clntSocket, buffer, numBytesRcved, 0);
        if(numBytesSent < 0)
            Deal_System_Error("send faild!");
        else if(numBytesSent != numBytesRcved)
            Deal_User_Error("send ","sent unexpected number of bytes");
        fputc('\n',stdout);
    }
    close(clntSocket);
}

int SetupTCPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));

    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    
    if(rtnVal != 0)
        Deal_User_Error("getaddrinfo()", "faild");
    
    int servSock = -1;

    for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(servSock < 0)
            continue;
        
        if((bind(servSock,addr->ai_addr,addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0))
        {
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            if(getsockname(servSock, (struct sockaddr *)&localAddr,&addrSize) < 0)
                Deal_System_Error("getsockname() faild!");
            fputs("Binding to ",stdout);
            PrintSockAddress((struct sockaddr *)&localAddr, stdout);
            break;
        }

        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        Deal_System_Error("accept() faild!");
    
    fputs("Handling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}