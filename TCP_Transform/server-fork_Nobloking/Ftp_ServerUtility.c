#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "Ftp_ServerUtility.h"
#include "Deal_Error.h"
#include "AddressUtility.h"
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

int SetupTCPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));

    addrCriteria.ai_family = AF_UNSPEC;     //any address family
    addrCriteria.ai_flags = AI_PASSIVE;     //accept on any address/port
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    
    if(rtnVal != 0)
        User_Error_Exit("getaddrinfo()", "faild");
    
    int servSock = -1;

    for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(servSock < 0)
            continue;
        
        if((bind(servSock,addr->ai_addr,addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0))
        {
            fputs("Binding to ",stdout);
            PrintSockAddress(addr->ai_addr,stdout);
            break;
        }
        
        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

//sock套接字本地端的IP地址和端口
int Get_Sock_Name(int sock_fd)  
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getsockname() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

//sock套接字连接端的IP地址和端口
int Get_Peer_Name(int sock_fd) 
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getpeername(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        System_Error_Exit("getpeername() faild!");

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

//获取客户端的套接字
int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        System_Error_Exit("accept() faild!");
    
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        System_Error_Exit("send faild");
    else if (Send_Bytes != buf_len)
        User_Error_Exit("send","sent unexpected number of bytes");

    return Send_Bytes;
}

//读取接收缓存区，并处理异常情况
//当连接关闭时，返回0，没有接收到数据时，阻塞，接收到数据时，返回读取的字节数
ssize_t recv_tcp(int sock_fd, void *buf)
{
    ssize_t numBytes;
    numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
    if(numBytes < 0)
        System_Error_Exit("recv() faild\n");
    else if(numBytes == 0)
    {
        Get_Peer_Name(sock_fd);
        printf("Connection disconneted\n");
    }    
    return numBytes;
}

//从sock中读取接收到的数据
//buf_len = 0时，读一次，最多读满一个BUFFSIZE
//buf_len > 0时，读到buf_len的大小为止
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv_tcp(sock_fd, buf);
        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv_tcp(sock_fd, buf);;
            totalBytesRcvd += numBytes;
        }
        return totalBytesRcvd;
    }   
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
    char Cmd_Buffer[BUFFSIZE];
    ssize_t numBytesRcved = 1;
    static FILE *tmp_fp;
    
    char *token;
    char Cmd[SERVICE_LEN],Parameter[3][SERVICE_LEN];


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
        token = strtok(Cmd_Buffer, DELIM);
        strcpy(Cmd,token);

        if(strcmp(Cmd,"down") == 0)  //执行用户命令
        {
            token = strtok(NULL, DELIM);
            if(token != NULL )
                strcpy(Parameter[0],token);
            else
                Send_Messege (str, "Download Parameter is wrong : down <source> <target>");
                
            token = strtok(NULL, DELIM);
            if(token != NULL)
                strcpy(Parameter[1],token);
            else
                Send_Messege (str, "Download Parameter is wrong : down <source> <target>");
            printf("Cmd: %s\nParameter[1]: %s\nParameter[2]: %s\n", Cmd, Parameter[0], Parameter[1]);
        }
        else  //执行系统命令
        {
            strcat(Cmd_Buffer,CMD_TMP_PATH);    //命令包装，加入输出重定向到文件
            system(Cmd_Buffer);                 //执行命令
            Send_File(str, TMP_Path);       //传输保存有命令执行结果的文件
        }
    }
    fclose(str); //flushes stream and closes socket
}