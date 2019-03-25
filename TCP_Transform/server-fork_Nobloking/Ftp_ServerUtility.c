#include <sys/wait.h>
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "Ftp_ServerUtility.h"
#include "Trans_Protocol.h"

//获取服务端配置
void Get_Server_Config(char *service, int argc, char **argv)
{
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char *token;
    
    if(argc ==2)  //优先读取命令行参数
        strcpy(service, argv[1]);
    else if(argc == 1) //使用默认参数，即读取配置文件
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            User_Error_Exit("fopen()", "connect.config not exit!");

        fgets(addrBuffer, SERVICE_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(service,token);

        fclose(Config_file_fp);
    }
    else
        User_Error_Exit("wrong arguments","<Server Port>");
}

//清理僵尸进程
void Clean_Zombies_Process(int *childProcCount)
{
    int processID = waitpid((pid_t) -1, NULL, WNOHANG);//Non=blocking wait
    if(processID < 0)
        System_Error_Exit("waitpid() faild");
    else if(processID > 0)
        (*childProcCount)--;
}

void HandleTCPClient(int clntSocket)
{
    char Cmd_Buffer[BUFFSIZE], Cmd[BUFFSIZE];
    ssize_t numBytesRcved = 1;
    static FILE *tmp_fp;
    
    char *token;
    char Parameter[3][SERVICE_LEN];


    //使用流包装套接字
    FILE *str = fdopen(clntSocket, "r+");
    if(str == NULL)
        System_Error_Exit("fdopen(clntSocket) faild");

    //创建一个临时文件用于保存客户端命令在服务端执行的结果
    if((tmp_fp = fopen(TMP_Path,"w+")) == NULL)
        User_Error_Exit("fopen(TMP_Path) error", "creat temp failed");
    fclose(tmp_fp);

    while(numBytesRcved > 0)
    {
        numBytesRcved = Recv_Messege(str, Cmd_Buffer); //获取指令
        //链接断开
        if(numBytesRcved == 0)
        {
            Get_Peer_Name(clntSocket);
            printf("Connection disconneted\n");
            break;
        }
        strcpy(Cmd, Cmd_Buffer);

        Bool Cmd_sta = Deal_Cmd(Cmd_Buffer, Parameter, 3);

        if(strcmp(Parameter[0],"down") == 0)  //执行用户命令
        {
            int Send_Bytes = Send_File(str, Parameter[1]);
            printf("Send %d Bytes\n", Send_Bytes);
        }
        else  //执行系统命令
        {
            strcat(Cmd,CMD_TMP_PATH);    //命令包装，加入输出重定向到文件
            system(Cmd);                 //执行命令
            Send_File(str, TMP_Path);       //传输保存有命令执行结果的文件
        }
    }
    fclose(str); //flushes stream and closes socket
}