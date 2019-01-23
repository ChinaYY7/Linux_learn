#include "TCPServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h"

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
            fputs("Binding to ",stdout);
            PrintSockAddress(addr->ai_addr,stdout);
            break;
        }
        
        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int Get_Sock_Name(int sock_fd)
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        Deal_System_Error("getsockname() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        Deal_System_Error("accept() faild!");
    
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        Deal_System_Error("send faild");
    else if (Send_Bytes != buf_len)
        Deal_User_Error("send","sent unexpected number of bytes");

    return Send_Bytes;
}

int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv(sock_fd, buf, BUFFIZE - 1, 0);
        if(numBytes < 0)
            Deal_System_Error("recv() faild\n");
        else if(numBytes == 0)
            printf("\nconnection closed prematurely\n");

        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv(sock_fd, buf, BUFFIZE - 1, 0);
            if(numBytes < 0)
                Deal_System_Error("recv() faild\n");
            else if(numBytes == 0)
                Deal_User_Error("recv", "connection closed prematurely");
            totalBytesRcvd += numBytes;
        }

        return totalBytesRcvd;
    }   
}

void HandleTCPClient(int clntSocket)
{
    char buffer[BUFFIZE];
    ssize_t numBytesRcved = 1;
    ssize_t numBytesSent;
    
    while(numBytesRcved > 0)
    {
        numBytesRcved = TCP_nReceive(clntSocket, buffer, 0);
        if(numBytesRcved != 0)
        {
            buffer[numBytesRcved - 1] = '\0';
            printf("\nReceived from client(%lu bytes): %s\n", numBytesRcved, buffer);

            numBytesSent = TCP_nSend(clntSocket, buffer, numBytesRcved);
            printf("  send    to  client(%lu bytes): %s\n", numBytesSent, buffer);
        }
    }
    close(clntSocket);
}