#include "TCPServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
void HandleTCPClient(int clntSocket)
{
    char buffer[BUFFIZE];
    ssize_t numBytesRcved = 1;
    ssize_t numBytesSent;
    
    while(numBytesRcved > 0)
    {
        numBytesRcved = recv(clntSocket, buffer, BUFFIZE - 1, 0);
        if(numBytesRcved < 0)
            Deal_System_Error("recv faild!");
        printf("%ld bytes have been received\n", numBytesRcved);
        buffer[numBytesRcved] = '\0';
        printf("Recived from client: %s\n",buffer);

        numBytesSent = send(clntSocket, buffer, numBytesRcved, 0);
        if(numBytesSent < 0)
            Deal_System_Error("send faild!");
        else if(numBytesSent != numBytesRcved)
            Deal_User_Error("send ","sent unexpected number of bytes");
        fputc('\n',stdout);
    }
    close(clntSocket);
}