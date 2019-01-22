#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>
#include "UDPServerUtility.h"
#include "Deal_Error.h"
#include "AddressUtility.h"

int main(int argc, char *argv[])
{
    if(argc != 2)
        Deal_User_Error("wrong arguments","<Server Port/Service>");
    
    char *service = argv[1];
    int servSock = SetupUDPServerSocket(service);

    if (servSock < 0)
        Deal_User_Error("SetupUDPServerSocket() faild ",service);

    for(;;)
    {
        HandleUDPClient(servSock);
    }
}