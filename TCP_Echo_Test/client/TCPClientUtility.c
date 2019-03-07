#include "TCPClientUtility.h"
#include "Deal_Error.h"
#include "AddressUtility.h" 

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
        Deal_User_Error("getaddrinfo()", "faild",ERROR_VALUE);

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
    
    freeaddrinfo(severAddr);
    return sock;
}

int Get_Sock_Name(int sock_fd)
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        Deal_System_Error("getsockname() faild!",ERROR_VALUE);

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        Deal_System_Error("send faild",ERROR_VALUE);
    else if (Send_Bytes != buf_len)
        Deal_User_Error("send","sent unexpected number of bytes",ERROR_VALUE);

    return Send_Bytes;
}

int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
        if(numBytes < 0)
            Deal_System_Error("recv() faild\n",ERROR_VALUE);
        else if(numBytes == 0)
            printf("\nconnection closed prematurely\n");
        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
            if(numBytes < 0)
                Deal_System_Error("recv() faild\n",ERROR_VALUE);
            else if(numBytes == 0)
                Deal_User_Error("recv", "connection closed prematurely",ERROR_VALUE);
            
            totalBytesRcvd += numBytes;
        }

        return totalBytesRcvd;
    }   
}