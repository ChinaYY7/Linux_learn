#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "Ftp_ServerUtility.h"
#include <sys/wait.h>
#include "AddressUtility.h"
#include <sys/shm.h>

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "


int main(int argc, char *argv[])
{
    //---------------------------------------------------------------------------------------------------
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char service[SERVICE_LEN];
    char *token;

    if(argc ==2)  //优先读取命令行参数
        strcpy(service, argv[1]);
    else if(argc == 1) //使用默认参数，即读取配置文件
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVICE_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(service,token);
    }
    else
        Deal_User_Error("wrong arguments","<Server Port>", ERROR_VALUE);
    //-----------------------------------------------------------------------------------------------------
    
    int servSock = SetupTCPServerSocket(service);  //创建服务端监听TCP套接字

    if(servSock < 0)
        Deal_System_Error("SetupTCPServerSocket",ERROR_VALUE);

    if (fcntl(servSock, F_SETFL, O_NONBLOCK) < 0)//设置非阻塞
        Deal_System_Error("Unable to put client sock into non-blocking/async mode", ERROR_VALUE);

    if (servSock < 0)
        Deal_User_Error("SetupTCPServerSocket() faild ",service, ERROR_VALUE);

    int clntSock;
    unsigned int childProcCount = 0;
    pid_t processID;
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    for(;;)
    {
        clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
        if(clntSock < 0)
        {
            if(errno != EWOULDBLOCK)
                Deal_System_Error("accept faild!",ERROR_VALUE);
        }
        else
        {
            //Fork child process and report any errors
            processID = fork();
            if(processID < 0)
                Deal_System_Error("fork() faild",ERROR_VALUE);
            else if(processID == 0)
            {
                close(servSock);
                Get_Peer_Name(clntSock);
                HandleTCPClient(clntSock);
                exit(0);
            }
            close(clntSock);
            childProcCount++;
        }
        if(childProcCount > 0)
            Clean_Zombies_Process(&childProcCount);
    }
}