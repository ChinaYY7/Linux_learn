#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>
#include <sys/file.h>
#include <signal.h>
#include "UDPServerUtility.h"
#include "Deal_Error.h"
#include "AddressUtility.h"

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

void UseIdleTime();
void SIGIOHandler(int signalType);

int servSock;

int main(int argc, char *argv[])
{
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char service[SERVICE_LEN];
    char *token;

    if(argc ==2)
        strcpy(service, argv[1]);
    else if(argc == 1)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVICE_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(service,token);
    }
    else
        Deal_User_Error("wrong arguments","<Server Port>", ERROR_VALUE);
    
    servSock = SetupUDPServerSocket(service);

    if (servSock < 0)
        Deal_User_Error("SetupUDPServerSocket() faild ",service, ERROR_VALUE);

    struct sigaction handler;

    handler.sa_handler = SIGIOHandler;
    if(sigfillset(&handler.sa_mask) < 0)
        Deal_System_Error("sigfillset() failed", ERROR_VALUE);

    handler.sa_flags = 0;

    if(sigaction(SIGIO,&handler, 0) < 0)
        Deal_System_Error("sigaction() faild for SIGIO", ERROR_VALUE);

    if(fcntl(servSock, __F_SETOWN, getpid()) < 0)
        Deal_System_Error("Unable to set process owner to us", ERROR_VALUE);
    
    if (fcntl(servSock, F_SETFL, O_NONBLOCK | FASYNC) < 0)
        Deal_System_Error("Unable to put client sock into non-blocking/async mode", ERROR_VALUE);

    for(;;)
    {
        //HandleUDPClient(servSock);
        UseIdleTime();
    }
}

void UseIdleTime()
{
    puts(".");
    sleep(3);
}

void SIGIOHandler(int signalType)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    char buffer[MAXSTRLEN + 1];
    ssize_t numBytesRcvd, numBytesSent;

    do
    {
        numBytesRcvd = recvfrom(servSock, buffer, MAXSTRLEN, 0, (struct sockaddr*)&clntAddr, &clntAddrLen);
        if(numBytesRcvd < 0)
        {
            if(errno != EWOULDBLOCK)
                Deal_System_Error("recvfrom() faild", ERROR_VALUE);
        }
        else
        {
            fputs("\nHandling client ", stdout);
            PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
            buffer[numBytesRcvd-1] = '\0'; //有换行符
            printf("Received from client(%lu bytes): %s\n", numBytesRcvd, buffer); 

            numBytesSent = UDP_Send_To(servSock,buffer,numBytesRcvd,&clntAddr, clntAddrLen);
            printf("  send    to  client(%lu bytes): %s\n", numBytesSent, buffer);
        }
    } 
    while (numBytesRcvd >= 0);
} 