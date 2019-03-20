#include "Ftp_ClientUtility.h"
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
    
    freeaddrinfo(severAddr);
    return sock;
}

int Get_Sock_Name(int sock_fd)
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getsockname() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
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

int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
        if(numBytes < 0)
            System_Error_Exit("recv() faild\n");
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
                System_Error_Exit("recv() faild\n");
            else if(numBytes == 0)
                User_Error_Exit("recv", "connection closed prematurely");
            
            totalBytesRcvd += numBytes;
        }

        return totalBytesRcvd;
    }   
}

char TCP_Receive_char(int sock_fd)
{
    ssize_t numBytes;
    char ch;
    numBytes = recv(sock_fd, &ch, 1, 0);
    if(numBytes < 0)
    {
        if(errno != EWOULDBLOCK)
            System_Error_Exit("accept faild!");
        else
            ch = -1;
    }
    if(ch != -1)
        //printf("ch = %c \n", ch);
    return ch;
}

int TCP_get_string(int sock_fd, char *str)
{
    ssize_t numBytes = 0;
    str[numBytes] = TCP_Receive_char(sock_fd);
    //if(str[numBytes] != -1)
        //printf("numBytes = %ld, ch = %c\n", numBytes,str[numBytes]);
    while(1)
    {
        if(str[numBytes]  == '\n')
        {
            //printf("read a ENTER\n");
            break;
        }
        else if(str[numBytes]  == -1)
        {
            //printf("read a END\n");
            break;
        }
        numBytes++;
        str[numBytes] = TCP_Receive_char(sock_fd);
        //printf("numBytes = %ld, ch = %c\n", numBytes,str[numBytes]);
        //sleep(1);
    }
    return numBytes;
}

ssize_t TCP_get_all(int sock_fd)
{
    char buffer[BUFFSIZE];
    ssize_t numBytes = 0;
    ssize_t sumBytes = 0;
    numBytes = TCP_get_string(sock_fd, buffer);
    //printf("numBytes = %ld\n",numBytes);
    sumBytes+=numBytes;
    if(numBytes > 0)
        printf("%s", buffer);
    while(numBytes != 0)
    {
        numBytes = TCP_get_string(sock_fd, buffer);
        //printf("numBytes = %ld\n",numBytes);
        sumBytes+=numBytes;
        if(numBytes > 0)
            printf("%s", buffer);
        //sleep(1);
    }
    //printf("sumBytes = %ld\n",sumBytes);
    //sleep(1);
    return sumBytes;
}