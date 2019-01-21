#include "TCPClientUtility.h"
#include "Deal_Error.h"

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
        Deal_User_Error("getaddrinfo()", "faild");

    int sock = -1;
    
    for(struct addrinfo *addr = severAddr; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(sock < 0)
            continue;
        if(connect(sock, addr->ai_addr,addr->ai_addrlen) == 0)
            break;
        close(sock);
        sock = -1;
    }
    
    freeaddrinfo(severAddr);
    return sock;
}