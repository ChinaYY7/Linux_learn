#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPServerUtility.h"
#include <pthread.h>

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

void *ThreadMain(void *arg);

struct ThreadArgs
{
    int clntSock;
};

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
    
    int servSock = SetupTCPServerSocket(service);

    if (servSock < 0)
        Deal_User_Error("SetupTCPServerSocket() faild ",service, ERROR_VALUE);

    for(;;)
    {
        int clntSock = AcceptTCPConnection(servSock);
        if(clntSock < 0)
            Deal_System_Error("accept faild!",ERROR_VALUE);

        //Create sparete memory for client argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs*) malloc(sizeof(struct ThreadArgs));

        if(threadArgs == NULL)
            Deal_System_Error("malloc() faild", ERROR_VALUE);

        threadArgs->clntSock = clntSock;

        //Create client thread
        pthread_t threadID;
        int returnValue = pthread_create(&threadID,NULL,ThreadMain,threadArgs);
        if(returnValue != 0)
            Deal_System_Error("pthread_create() faild", ERROR_VALUE);
        
        printf("with thread %lu\n", (unsigned long int) threadID);
    }
}

void *ThreadMain(void *threadArgs)
{
    //线程返回后立即释放线程状态
    pthread_detach(pthread_self());

    int clntSock = ((struct ThreadArgs *) threadArgs)->clntSock;
    free(threadArgs);

    HandleTCPClient(clntSock);
    return (NULL);
}