#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPServerUtility.h"
#include <sys/time.h>

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

#define SERVICE_NUM 5
#define TIMEOUT 5

int main(int argc, char *argv[])
{
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char service[SERVICE_NUM][SERVICE_LEN];
    char *token;

    int noPorts = 0;
    int i;

    if(argc > 1)
    {
        while(noPorts < argc - 1)
        {
            strcpy(service[noPorts], argv[noPorts + 1]);
            noPorts++;
        }
    }
    else if(argc == 1)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVICE_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        while(token != NULL || noPorts < SERVICE_NUM)
        {
            strcpy(service[noPorts],token);
            noPorts++;
            token = strtok(NULL, DELIM);
        }
    }
    else
        Deal_User_Error("wrong arguments","<Server Port>", ERROR_VALUE);

    int servSock[noPorts];
    int maxDescriptor = 0;
    
    for(i = 0; i < noPorts; i++)
    {
        servSock[i] = SetupTCPServerSocket(service[i]);
        if (servSock[i] < 0)
            Deal_User_Error("SetupTCPServerSocket() faild ",service[i], ERROR_VALUE);
        if(servSock[i] > maxDescriptor)
            maxDescriptor = servSock[i];
    }

    puts("Starting server: Hit return to shutdown");
    Bool running = True;
    fd_set sockSet;
    long timeout = TIMEOUT;
    int clntSock;

    while(running)
    {
        FD_ZERO(&sockSet);
        FD_SET(STDIN_FILENO, &sockSet);
        for(i = 0; i < noPorts; i++)
            FD_SET(servSock[i], &sockSet);

        struct timeval selTimeout;
        selTimeout.tv_sec = timeout;
        selTimeout.tv_usec = 0;

        //suspend program until descriptor is ready or timeout
        if(select(maxDescriptor + 1, &sockSet, NULL, NULL, &selTimeout) == 0)
            printf("No echo requests for %ld secs ... Server still alive\n", timeout);
        else
        {
            if(FD_ISSET(0, &sockSet))
            {
                puts("Shutting down server");
                getchar();
                running =False;
            }
            for (i = 0; i < noPorts; i++)
            {
                if(FD_ISSET(servSock[i],&sockSet))
                {
                    printf("Request on port %s: ", service[i]);
                    clntSock = AcceptTCPConnection(servSock[i]);
                    if(clntSock < 0)
                        Deal_System_Error("accept faild!",ERROR_VALUE);

                    HandleTCPClient(clntSock);
                }
            }
        }
        
    }

    for(i = 0; i < noPorts; i++)  
        close(servSock[i]);

    exit(0);
}