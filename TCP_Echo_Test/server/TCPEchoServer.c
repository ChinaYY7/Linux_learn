#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPServerUtility.h"

int main(int argc, char *argv[])
{
    if(argc != 2)
        Deal_User_Error("wrong arguments","<Server Port>");
    
    char *service = argv[1];
    int servSock = SetupTCPServerSocket(service);

    if (servSock < 0)
        Deal_User_Error("SetupTCPServerSocket() faild ",service);

    for(;;)
    {
        int clntSock = AcceptTCPConnection(servSock);
        if(clntSock < 0)
            Deal_System_Error("accept faild!");

        HandleTCPClient(clntSock);
        close(clntSock);
    }
}