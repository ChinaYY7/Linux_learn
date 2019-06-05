#include <sys/wait.h>
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "Ftp_ServerUtility.h"
#include "Trans_Protocol.h"
#include <sys/time.h> 

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
    int processID;
    while(1)
    {
        processID = waitpid((pid_t) -1, NULL, WNOHANG);//Non=blocking wait
        if(processID < 0)
            System_Error_Exit("waitpid() faild");
        else if(processID > 0)
            (*childProcCount)--;
        else
            break;
    }  
}

void HandleTCPClient(int clntSocket)
{
    char Cmd_Buffer[BUFFSIZE], Cmd[BUFFSIZE];
    ssize_t numBytesRcved = 1;
    static FILE *tmp_fp;

    struct timeval start,finish;
    double speed,Complate_time;
    
    char *token;
    char Parameter[3][SERVICE_LEN];
    
    //使用流包装套接字
    FILE *Cmd_Str = fdopen(clntSocket, "r+");
    if(Cmd_Str == NULL)
        System_Error_Exit("fdopen(clntSocket) faild");

    //创建服务端连接客户端的套接字
    char Date_Port[6];
    numBytesRcved = Recv_Messege(Cmd_Str, Date_Port); //获取指令
    //链接断开
    if(numBytesRcved == FREAD_EOF)
    {
        Get_Peer_Name(clntSocket);
        printf(" Connection disconneted\n");
        fclose(Cmd_Str); //flushes stream and closes socket
        return;
    }
    printf("\nServer->Client: ");
    char addrBuffer[INET6_ADDRSTRLEN];
    Get_Address(clntSocket, addrBuffer);
    int Date_Sock = SetupTCPClientSocket(addrBuffer, Date_Port);
    if (Date_Sock < 0)
        User_Error_Exit("SetupTCPClientSocket(Date_Sock) faild!", "Unable to connect");

    //使用流包装套接字
    FILE *Date_Str = fdopen(Date_Sock, "r+");
    if(Date_Str == NULL)
        System_Error_Exit("fdopen(Date_Sock) faild");

    //创建一个临时文件用于保存客户端命令在服务端执行的结果
    if((tmp_fp = fopen(TMP_Path,"w+")) == NULL)
        User_Error_Exit("fopen(TMP_Path) error", "creat temp failed");
    fclose(tmp_fp);

    while(numBytesRcved > 0)
    {
        numBytesRcved = Recv_Messege(Cmd_Str, Cmd_Buffer); //获取指令
        //链接断开
        if(numBytesRcved == FREAD_EOF)
        {
            Get_Peer_Name(clntSocket);
            printf(" Connection disconneted\n");
            break;
        }
        strcpy(Cmd, Cmd_Buffer);

        Bool Cmd_sta = Deal_Cmd(Cmd_Buffer, Parameter, 3);

        if(strcmp(Parameter[0],"down") == 0)  //执行用户命令
        {
            //printf("Recived dowm cmd[%s %s %s]\n", Parameter[0], Parameter[1], Parameter[2]);
            gettimeofday(&start,NULL);
            sleep(1);
            int Send_Bytes = Send_File(Date_Str, Parameter[1]);
            if(Send_Bytes == FOPEN_ERROR)
                Send_Messege(Cmd_Str,"Target file not exit in server !");
            else if(Send_Bytes == FWRITE_ERROR)
                Send_Messege(Cmd_Str,"Read Target file error !");
            else
            {
                gettimeofday(&finish,NULL);
                Complate_time = (double)((finish.tv_sec-start.tv_sec) * 1000000 + (finish.tv_usec-start.tv_usec)) / 1000000;
                speed = Send_Bytes / 1024 / 1024 / Complate_time; //MB/S
                printf("\nSend %d Bytes and Speed: %.2f MB/s\n", Send_Bytes, speed);
                printf("send Complete\n");
            }
        }
        else  //执行系统命令
        {
            strcat(Cmd,CMD_TMP_PATH);    //命令包装，加入输出重定向到文件
            system(Cmd);                 //执行命令
            Send_File(Cmd_Str, TMP_Path);       //传输保存有命令执行结果的文件
        }
    }
    fclose(Cmd_Str); //flushes stream and closes socket
    fclose(Date_Str);
}