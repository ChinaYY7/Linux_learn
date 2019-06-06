#include <sys/wait.h>
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "M_ServerUtility.h"
#include "Trans_Protocol.h"
#include <sys/time.h> 
static char buffer[1024];


static Slave_Server_Address Slave_Server_Address_Tbl[TBL_SIZE];

//初始化服务器地址表
void Init_Slave_Server_Address_Tbl(void)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        Slave_Server_Address_Tbl[i].Enable = False;
        Slave_Server_Address_Tbl[i].load = 0;
    }
}

//将子服务器地址插入服务器地址表
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
            Slave_Server_Address_Tbl[i].load = 0;
            //printf("Sock(%d) Insert Slave_Server_Address_Tbl[%d]\n", Sock, i);
            break;
        }     
    }
    if(i == TBL_SIZE)
        return -1;
    return 0;
}

//将子服务器地址从服务器地址表中删除
int Delete_Slave_Server_Address(int Sock)
{
    int i;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Sock == Sock)
        {
            Slave_Server_Address_Tbl[i].Enable = False;
            //printf("Sock(%d) be delete from Slave_Server_Address_Tbl[%d]\n", Sock, i);
        } 
    }
    if(i == TBL_SIZE)
        return -1;
    return 0;
}

//判断服务器地址表是否为空
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


//从服务器地址表中选择一个服务器地址
int Select_Server_Address(void)
{
    int i;
    static int min_load = 1;
    int select = -1;
    for(i = 0; i < TBL_SIZE; i++)
    {
        if(Slave_Server_Address_Tbl[i].Enable == True)
        {
            if(Slave_Server_Address_Tbl[i].load <= min_load)
            {
                min_load = Slave_Server_Address_Tbl[i].load;
                select = i;
            }
        }   
    }
    if(select < 0)
        return -1;
    Slave_Server_Address_Tbl[select].load++;
    min_load++;
    printf("min_load = %d\n",min_load);
    return select;
}

//屏蔽SIGPIPE信号
int Shield_SIGPIPE(void)
{
    struct sigaction handler;

    handler.sa_handler = SIG_IGN;
    if(sigfillset(&handler.sa_mask) < 0)
    {
        System_Error("sigfillset() failed");
        return -1;
    } 
    handler.sa_flags = 0;
    if(sigaction(SIGPIPE,&handler, 0) < 0)
    {
        System_Error("sigaction() faild for SIGPIPE");
        return -2;
    }
    
    return 0;
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

//与子服务器的通讯
int Talk_Slave(int Slave_Sock)
{
    ssize_t numBytesRcved;
    static int First_Sta = 1;
    FILE *Slave_Str;
    
    //使用流包装套接字
    Slave_Str = fdopen(Slave_Sock, "r+");
    if(Slave_Str == NULL)
        System_Error_Exit("fdopen(Slave_Sock) faild");

    char Browser_Port[6];
    numBytesRcved = Recv_Messege(Slave_Str, Browser_Port); //获取指令
    //链接断开
    if(numBytesRcved == FREAD_EOF)
    {
        Get_Peer_Name(Slave_Sock);
        printf(" Disconnected\n");
        Delete_Slave_Server_Address(Slave_Sock);
        fclose(Slave_Str);//flushes stream and closes socket
        return -1;
    }

    char addrBuffer[INET6_ADDRSTRLEN];
    Get_Address(Slave_Sock, addrBuffer);
    strcat(addrBuffer,":");
    strcat(addrBuffer,Browser_Port);

    printf("The slave address is: %s\n", addrBuffer);
    Insert_Slave_Server_Address(addrBuffer, Slave_Sock);

    return 0;
}

const char Default_Address[20] = "www.baidu.com";

//回应浏览器的请求
void Reponse_Browser(int Browser_Sock)
{
    ssize_t numBytesSent;

    printf("Recevie A Browser's Request\n");
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "HTTP/1.1 302 Moved Temporarily\r\n");
    strcat(buffer, "Location: http://");

    if(Slave_Server_Address_Empty())
    {
        strcat(buffer, Default_Address);
        printf("Browser connect to default adress: %s\n",Default_Address);
    }
    else
    {
        int Select_Num = Select_Server_Address();
        if(Select_Num < 0)
        {
            printf("Select error\n");
        }
        else
        {
            strcat(buffer, Slave_Server_Address_Tbl[Select_Num].address);
            printf("Browser connect to Slave_Server: %s\n", Slave_Server_Address_Tbl[Select_Num].address);
        }  
        
    }
    
    numBytesSent = TCP_nSend(Browser_Sock, buffer, strlen(buffer));

    close(Browser_Sock);
}