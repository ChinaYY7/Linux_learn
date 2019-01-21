#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPServerUtility.h"

static const int MAXPENDING = 5;

int main(int argc, char *argv[])
{
    if(argc != 2)
        Deal_User_Error("wrong arguments","<Server Port>");
    
    in_port_t servPort = atoi(argv[1]);
    int servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (servSock < 0)
        Deal_System_Error("socket() faild!");

    struct sockaddr_in servAddr;
    memset(&servAddr, 0 , sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    if(bind(servSock, (struct sockaddr*) &servAddr,sizeof(servAddr)) < 0)
        Deal_System_Error("bind faild!");

    if(listen(servSock, MAXPENDING) < 0)
        Deal_System_Error("listen faild!");

    for(;;){
        struct sockaddr_in clntAddr;
        socklen_t clntAddrLen = sizeof(clntAddr);

        int clntSock = accept(servSock, (struct sockaddr*) &clntAddr, &clntAddrLen);
        if(clntSock < 0)
            Deal_System_Error("accept faild!");

        char clntName[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, sizeof(clntName)) != NULL)
            printf("\nHanding clint %s/%d\n", clntName, ntohs(clntAddr.sin_port));
        else
            puts("Unable to get client address");
        HandleTCPClient(clntSock);
    }
}