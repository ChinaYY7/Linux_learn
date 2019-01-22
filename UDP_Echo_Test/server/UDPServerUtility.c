#include "UDPServerUtility.h"

int SetupUDPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));

    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

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
        
        if(bind(servSock,addr->ai_addr,addr->ai_addrlen) == 0)
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

int UDP_Recive_From(int servSock, void *buffer, struct sockaddr_storage *clntAddr, socklen_t *clntAddrLen)
{
    ssize_t numBytesRcvd;

    numBytesRcvd = recvfrom(servSock, buffer, MAXSTRLEN, 0, (struct sockaddr*)clntAddr, clntAddrLen);
    if(numBytesRcvd < 0)
        Deal_System_Error("recvfrom() faild ");
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)clntAddr, stdout);
    return numBytesRcvd;
}

int UDP_Send_To(int servSock, const void *buffer, size_t send_num, struct sockaddr_storage *clntAddr, socklen_t clntAddrLen)
{
    ssize_t numBytesSent;

    numBytesSent = sendto(servSock, buffer, send_num, 0, (struct sockaddr *)clntAddr, clntAddrLen);
    if(numBytesSent < 0)
        Deal_System_Error("sendto() faild ");
    else if(numBytesSent != send_num)
        Deal_User_Error("sendto() faild ","sent unexoected number of bytes");
    return numBytesSent;
}

void HandleUDPClient(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    char buffer[MAXSTRLEN + 1];
    ssize_t numBytesRcvd, numBytesSent;

    numBytesRcvd = UDP_Recive_From(servSock,buffer,&clntAddr, &clntAddrLen);
    buffer[numBytesRcvd-1] = '\0'; //有换行符
    printf("Received from client(%lu bytes): %s\n", numBytesRcvd, buffer);

    numBytesSent = UDP_Send_To(servSock,buffer,numBytesRcvd,&clntAddr, clntAddrLen);
    printf("  send    to  client(%lu bytes): %s\n", numBytesSent, buffer);
}