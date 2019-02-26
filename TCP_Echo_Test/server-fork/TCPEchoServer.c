#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPServerUtility.h"
#include <sys/wait.h>

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

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

    int clntSock;
    unsigned int childProcCount = 0;
    pid_t processID;
    for(;;)
    {
        clntSock = AcceptTCPConnection(servSock);
        if(clntSock < 0)
            Deal_System_Error("accept faild!",ERROR_VALUE);

        //Fork child process and report any errors
        processID = fork();
        if(processID < 0)
            Deal_System_Error("fork() faild",ERROR_VALUE);
        else if(processID == 0)
        {
            close(servSock);
            HandleTCPClient(clntSock);
            exit(0);
        }
        printf("with child process: %d\n", processID);
        close(clntSock);
        childProcCount++;
        //clean up all zombies
        while(childProcCount)
        {
            processID = waitpid((pid_t) -1, NULL, WNOHANG);//Non=blocking wait
            if(processID < 0)
                Deal_System_Error("waitpid() faild", ERROR_VALUE);
            else if(processID == 0) //No zombil to wait on
                break;
            else
                childProcCount--;
        }
    }
}