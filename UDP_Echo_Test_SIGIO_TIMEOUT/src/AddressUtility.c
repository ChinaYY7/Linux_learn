#include "AddressUtility.h"
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
    fputc('\n',stdout);
}