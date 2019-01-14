#include "TCPServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
void HandleTCPClient(int clntSocket)
{
    char buffer[4096];
    ssize_t numBytesRcved = recv(clntSocket, buffer, 4095, 0);

    printf("numBytesRcved: %ld\n", numBytesRcved);
    if(numBytesRcved < 0)
        Deal_System_Error("recv faild!");
    
    while(numBytesRcved > 0)
    {
        ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcved, 0);
        if(numBytesSent < 0)
            Deal_System_Error("send faild!");
        else if(numBytesSent != numBytesRcved)
            Deal_User_Error("send ","sent unexpected number of bytes");

        numBytesRcved = recv(clntSocket, buffer, 4095, 0);
        printf("numBytesRcved: %ld\n", numBytesRcved);

        if(numBytesRcved < 0)
            Deal_System_Error("recv faild!");
    }

    close(clntSocket);
}