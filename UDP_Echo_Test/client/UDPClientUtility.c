#include "UDPClientUtility.h"
#include "Deal_Error.h"

int SetupUDPClientSocket(const char *host, const char *service, struct addrinfo **severAddr)
{
    struct addrinfo addCriteria;
    memset(&addCriteria, 0 , sizeof(addCriteria));

    addCriteria.ai_family = AF_UNSPEC;      //设置地址族，允许返回的地址来自于任何地址族（IPV4 or IPV6）
    addCriteria.ai_socktype = SOCK_DGRAM;  //设置套接字类型
    addCriteria.ai_protocol = IPPROTO_UDP;  //设置协议

    int rtnVal = getaddrinfo(host, service, &addCriteria, severAddr);
    if(rtnVal != 0)
        Deal_User_Error("getaddrinfo()", "faild");

    int sock = -1;
    
    for(struct addrinfo *addr = *severAddr; addr != NULL; addr = addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(sock < 0)
            continue;
        else
            break;
        close(sock);
        sock = -1;
    }

    return sock;
}

int SockAddrsEqual(struct sockaddr *Servaddr, struct sockaddr *Fromaddr)
{
    if(Servaddr->sa_family == Fromaddr->sa_family)
    {
        if(Servaddr->sa_family == AF_INET)
        {
            if(((struct sockaddr_in *)Servaddr)->sin_addr.s_addr == ((struct sockaddr_in *)Fromaddr)->sin_addr.s_addr)
                return 0;
            else
                return 1;
        }
        else if(Servaddr->sa_family == AF_INET6)
        {
            if(((struct sockaddr_in6 *)Servaddr)->sin6_addr.__in6_u.__u6_addr32 == ((struct sockaddr_in6 *)Fromaddr)->sin6_addr.__in6_u.__u6_addr32)
                return 0;
            else
                return 1;
        }
        else
            return 1;
    }
    else
        return 1;
}