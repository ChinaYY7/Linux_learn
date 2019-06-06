#include <sys/wait.h>
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "M_ServerUtility.h"
#include "Trans_Protocol.h"
#include <sys/time.h> 
static char buffer[1024];
static Slave_Server_Address Slave_Server_Address_Tbl[TBL_SIZE];

int Insert_Slave_Server_Address(const char *addrBuffer, int Sock)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Enable == False)
        {
            strcpy(Slave_Server_Address_Tbl[i].address,addrBuffer);
            Slave_Server_Address_Tbl[i].Sock = Sock;
            Slave_Server_Address_Tbl[i].Enable = True;
        }     
    }
    if(i == TBL_SIZE)
        return -1;
    return 0;
}

int Delete_Slave_Server_Address(int Sock)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Sock == Sock)
            Slave_Server_Address_Tbl[i].Enable = False;
    }
    if(i == TBL_SIZE)
        return -1;
    return 0;
}

int Slave_Server_Address_Empty(void)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Enable == True)
            return 0;    
    }
    return 1;
}

int Select_Server_Address(void)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Enable == True)
            return i;    
    }
    return -1;
}

//获取服务端配置
void Get_Server_Config(char *Chrom_Port, char *Slave_Port, int argc, char **argv)
{
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char *token;
    
    if(argc ==3)  //优先读取命令行参数
    {
        strcpy(Chrom_Port, argv[1]);
        strcpy(Slave_Port, argv[2]);
    }
        
    else if(argc < 3) //使用默认参数，即读取配置文件
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            User_Error_Exit("fopen()", "connect.config not exit!");

        fgets(addrBuffer, SERVICE_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(Chrom_Port,token);
        token = strtok(NULL, DELIM);
        strcpy(Slave_Port,token);

        fclose(Config_file_fp);
    }
    else
        User_Error_Exit("wrong arguments","<Chrom_Port Port> <Slave_Port Port>");
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

void Talk_Slave(int Slave_Sock)
{
    ssize_t numBytesRcved;
    static int First_Sta = 1;
    FILE *Slave_Str;
    
    //使用流包装套接字
    Slave_Str = fdopen(Slave_Sock, "r+");
    if(Slave_Str == NULL)
        System_Error_Exit("fdopen(Slave_Sock) faild");

    char Date_Port[6];
    numBytesRcved = Recv_Messege(Slave_Str, Date_Port); //获取指令
    //链接断开
    if(numBytesRcved == FREAD_EOF)
    {
        Get_Peer_Name(Slave_Sock);
        printf(" Connection disconneted\n");
        fclose(Slave_Str); //flushes stream and closes socket
        Delete_Slave_Server_Address(Slave_Sock);
        return;
    }

    char addrBuffer[INET6_ADDRSTRLEN];
    Get_Address(Slave_Sock, addrBuffer);
    strcat(addrBuffer,":");
    strcat(addrBuffer,Date_Port);

    printf("The address is: %s\n", addrBuffer);
    Insert_Slave_Server_Address(addrBuffer, Slave_Sock);
    //fclose(Slave_Str);
}

void Reponse_Chrom(int Chrom_Sock)
{
    ssize_t numBytesSent;

    printf("recv chrom request\n");
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "HTTP/1.1 302 Moved Temporarily\r\n");
    strcat(buffer, "Location: http://");

    if(Slave_Server_Address_Empty())
        strcat(buffer, "www.baidu.com\r\n\r\n");
    else
    {
        int Select_Num = Select_Server_Address();
        strcat(buffer, Slave_Server_Address_Tbl[Select_Num].address);
        printf("Chrom connect to Slave_Server: %s\n", Slave_Server_Address_Tbl[Select_Num].address);
    }
    

    numBytesSent = TCP_nSend(Chrom_Sock, buffer, strlen(buffer));

    close(Chrom_Sock);
}



/*
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
*/