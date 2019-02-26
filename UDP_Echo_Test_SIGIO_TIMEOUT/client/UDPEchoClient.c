#include "apue.h"
#include <sys/socket.h>
#include "Deal_Error.h"
#include "UDPClientUtility.h"

#define SERVER_LEN 100
#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

#define MAXSTRIES 5
#define TIMEOUT_SECS 2

unsigned int tries = 0;

void CatchAlarm(int ignored);

int main(int argc, char *argv[])
{
    FILE *Config_file_fp;
    char addrBuffer[SERVER_LEN];
    char server[SERVER_LEN], service[SERVICE_LEN];
    char *token;

    if(argc ==3)
    {
        strcpy(server, argv[1]);
        strcpy(service, argv[2]);
    }
    else if(argc == 1)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);
    }
    else
        Deal_User_Error("wrong arguments","<Server> <Port/Service>", ERROR_VALUE);

    struct addrinfo *severAddr;

    int sock = SetupUDPClientSocket(server, service, &severAddr);

    if (sock < 0)
        Deal_User_Error("SetupUDPClientSocket() faild!", "Unable to connect", ERROR_VALUE);
    
    //---------------set signal handler for alarm signal-----------------------
    struct sigaction handler;
    handler.sa_handler = CatchAlarm;
    if(sigfillset(&handler.sa_mask) < 0)
        Deal_System_Error("sigfillset() faild", ERROR_VALUE);
    handler.sa_flags = 0;

    if(sigaction(SIGALRM, &handler, 0) < 0)
        Deal_System_Error("sigaction() faild for SIGALRM", ERROR_VALUE);
    //-------------------------------------------------------------------------

    char echoString[MAXSTRLEN * 2]; 
    size_t echoStringLen;
    ssize_t numBytes;
    char buffer[MAXSTRLEN + 1];
    struct sockaddr_storage fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);

    while(1)
    {
        printf("\nPlease input sended string:   ");
        fgets(echoString, BUFFIZE, stdin);

        echoStringLen = strlen(echoString);
        if(echoStringLen > MAXSTRLEN)
            Deal_User_Error("echoString","String too long!", ERROR_VALUE);

        numBytes = sendto(sock, echoString, echoStringLen, 0, severAddr->ai_addr, severAddr->ai_addrlen);
        if(numBytes < 0)
            Deal_System_Error("sendto() faild", ERROR_VALUE);
        else if (numBytes != echoStringLen)
            Deal_User_Error("sendto() error","sent unexpected number of bytes", ERROR_VALUE);
        printf("send %lu bytes, try time :%d\n", numBytes, tries + 1);

        alarm(TIMEOUT_SECS);  //set alarm ,SIGALARM 将导致recvfrom()返回-1，并把errno设置为EINTR
        tries = 0;
        
        while((numBytes = recvfrom(sock, buffer, MAXSTRLEN, 0,(struct sockaddr *)&fromAddr, &fromAddrLen)) < 0)
        {
            if(errno == EINTR)
            {
                if(tries < MAXSTRIES)
                {
                    numBytes = sendto(sock, echoString, echoStringLen, 0, severAddr->ai_addr, severAddr->ai_addrlen);
                    if(numBytes < 0)
                        Deal_System_Error("sendto() faild", ERROR_VALUE);
                    else if (numBytes != echoStringLen)
                        Deal_User_Error("sendto() error","sent unexpected number of bytes", ERROR_VALUE);
                    printf("send %lu bytes, try time :%d\n", numBytes, tries + 1);
                    alarm(TIMEOUT_SECS); //重置alarm
                }
                else
                    Deal_User_Error("No Response", "unable to communication with server",ERROR_VALUE);
            }
            else
                Deal_System_Error("recvfrom() faild", ERROR_VALUE);
        }
        alarm(0); //cancel the timeout
    
        if(numBytes != echoStringLen)
            Deal_User_Error("recvfrom() error", "recevied unexpected number of bytes", ERROR_VALUE);

        if(!SockAddrsEqual(severAddr->ai_addr, (struct sockaddr *)&fromAddr))
            Deal_User_Error("recvfrom()", "recevied a packet from unknown source!", ERROR_VALUE);

        freeaddrinfo(severAddr);

        buffer[echoStringLen] = '\0';
        printf("Received from sever(%lu bytes): %s\n", numBytes, buffer);
    }
    
    close(sock);
    return 0;
} 

void CatchAlarm(int ignored)
{
    tries+=1;
}