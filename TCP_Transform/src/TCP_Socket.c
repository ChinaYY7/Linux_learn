#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "TCP_Socket.h"
#include "Deal_Error.h"

//建立客户端套接字
int SetupTCPClientSocket(const char *host, const char *service)
{
    struct addrinfo addCriteria;
    memset(&addCriteria, 0 , sizeof(addCriteria));

    addCriteria.ai_family = AF_UNSPEC;      //设置地址族，允许返回的地址来自于任何地址族（IPV4 or IPV6）
    addCriteria.ai_socktype = SOCK_STREAM;  //设置套接字类型
    addCriteria.ai_protocol = IPPROTO_TCP;  //设置协议

    struct addrinfo *severAddr;
    int rtnVal = getaddrinfo(host, service, &addCriteria, &severAddr);
    if(rtnVal != 0)
        User_Error_Exit("getaddrinfo()", "faild");

    int sock = -1;
    
    for(struct addrinfo *addr = severAddr; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(sock < 0)
            continue;
        if(connect(sock, addr->ai_addr,addr->ai_addrlen) == 0)
        {
            printf("connect to ");
            PrintSockAddress(addr->ai_addr,stdout);
            break;
        }
        perror("connect()faild");
        close(sock);
        sock = -1;
    }
    printf(" ......Completed\n");
    freeaddrinfo(severAddr);
    return sock;
}

//建立服务端监听套接字
int SetupTCPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));

    addrCriteria.ai_family = AF_UNSPEC;     //any address family
    addrCriteria.ai_flags = AI_PASSIVE;     //accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    
    if(rtnVal != 0)
        User_Error_Exit("getaddrinfo()", "faild");
    
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
            fputc('\n',stdout);
            break;
        }
        
        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        System_Error_Exit("send faild");
    else if (Send_Bytes != buf_len)
        User_Error_Exit("send","sent unexpected number of bytes");

    return Send_Bytes;
}

//读取接收缓存区，并处理异常情况
//当连接关闭时，返回0，没有接收到数据时，阻塞，接收到数据时，返回读取的字节数
ssize_t recv_tcp(int sock_fd, void *buf)
{
    ssize_t numBytes;
    numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
    if(numBytes < 0)
        System_Error_Exit("recv() faild\n");
    else if(numBytes == 0)
    {
        Get_Peer_Name(sock_fd);
        printf("Connection disconneted\n");
    }    
    return numBytes;
}

//从sock中读取接收到的数据
//buf_len = 0时，读一次，最多读满一个BUFFSIZE
//buf_len > 0时，读到buf_len的大小为止
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv_tcp(sock_fd, buf);
        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv_tcp(sock_fd, buf);;
            totalBytesRcvd += numBytes;
        }
        return totalBytesRcvd;
    }   
}
//获取客户端的套接字
int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        System_Error_Exit("accept() faild!");
    
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}

//打印地址
void PrintSockAddress(const struct sockaddr *address, FILE *stream)
{
    if(address == NULL || stream == NULL)
        return;

    void *numericAddress;
    char addrBuffer[INET6_ADDRSTRLEN];
    in_port_t ports;
    char IP_family_name[5];

    switch(address->sa_family)
    {
        case AF_INET:
            numericAddress = &((struct sockaddr_in*)address)->sin_addr;
            ports = ntohs(((struct sockaddr_in*)address)->sin_port);
            strcpy(IP_family_name,"IPv4");
            break;
        case AF_INET6:
            numericAddress = &((struct sockaddr_in6*)address)->sin6_addr;
            ports = ntohs(((struct sockaddr_in6*)address)->sin6_port);
            strcpy(IP_family_name,"IPv6");
            break;
        default:
            fputs("unknown type", stream);
            return;
    }

    if(inet_ntop(address->sa_family,numericAddress,addrBuffer,sizeof(addrBuffer)) == NULL)
        fputs("invaild address", stream);
    else
    {
        fprintf(stream,"%s %s",IP_family_name,addrBuffer);
        if(ports != 0) //Zero not vaild in any socket addr
            fprintf(stream,"-%u",ports);
    }
    //fputc('\n',stdout);
}

//获取IP地址
int Get_Address(int sock_fd, char *Buffer)
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getpeername(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getpeername() faild!");

    const struct sockaddr *address = (struct sockaddr *)&localAddr;
    void *numericAddress;
    char addrBuffer[INET6_ADDRSTRLEN];
    in_port_t ports;

    switch(address->sa_family)
    {
        case AF_INET:
            numericAddress = &((struct sockaddr_in*)address)->sin_addr;
            ports = ntohs(((struct sockaddr_in*)address)->sin_port);
            break;
        case AF_INET6:
            numericAddress = &((struct sockaddr_in6*)address)->sin6_addr;
            ports = ntohs(((struct sockaddr_in6*)address)->sin6_port);
            break;
        default:
            return -1;
    }

    if(inet_ntop(address->sa_family,numericAddress,addrBuffer,sizeof(addrBuffer)) == NULL)
        return -1;
    else
        strcpy(Buffer, addrBuffer);
}

//sock套接字本地端的IP地址和端口
int Get_Sock_Name(int sock_fd)  
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getsockname() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

//sock套接字连接端的IP地址和端口
int Get_Peer_Name(int sock_fd) 
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getpeername(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getpeername() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

