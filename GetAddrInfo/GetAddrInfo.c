#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h" 

int main(int argc, char *argv[])
{
    if(argc != 3)
        Deal_User_Error("wrong arguments","<Address/Name> <Port/Service>");
    
    char *addrString = argv[1];
    char *portString = argv[2];

    struct addrinfo addCriteria;
    memset(&addCriteria, 0 , sizeof(addCriteria));

    addCriteria.ai_family = AF_UNSPEC;      //设置地址族，允许返回的地址来自于任何地址族（IPV4 or IPV6）
    addCriteria.ai_socktype = SOCK_STREAM;  //设置套接字类型
    addCriteria.ai_protocol = IPPROTO_TCP;  //设置协议

    struct addrinfo *addrList;
    int rtnval = getaddrinfo(addrString,portString,&addCriteria,&addrList); //第二个参数提供0则只会打印地址信息
    if(rtnval != 0)
        Deal_User_Error("getaddrinfo()", "faild");
    
    for(struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next)
        PrintSockAddress(addr->ai_addr,stdout);

    freeaddrinfo(addrList);
    return 0;
} 