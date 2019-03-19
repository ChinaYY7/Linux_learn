#include "Ftp_ServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h"
#include <sys/wait.h>
#include "Trans_Protocol.h"
#include "Encode.h"
#include "Framer.h"

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
        Deal_User_Error("getaddrinfo()", "faild", ERROR_VALUE);
    
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

int Get_Sock_Name(int sock_fd)  //sock本地端的IP地址和端口
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        Deal_System_Error("getsockname() faild!",ERROR_VALUE);

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

int Get_Peer_Name(int sock_fd) //sock连接端的IP地址和端口
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getpeername(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        Deal_System_Error("getpeername() faild!",ERROR_VALUE);

    printf("\n");
    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        Deal_System_Error("accept() faild!",ERROR_VALUE);
    
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        Deal_System_Error("send faild",ERROR_VALUE);
    else if (Send_Bytes != buf_len)
        Deal_User_Error("send","sent unexpected number of bytes", ERROR_VALUE);

    return Send_Bytes;
}

int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
        if(numBytes < 0)
            Deal_System_Error("recv() faild\n",ERROR_VALUE);
        else if(numBytes == 0)
        {
            Get_Peer_Name(sock_fd);
            printf("connection closed prematurely\n");
        }    
        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
            if(numBytes < 0)
                Deal_System_Error("recv() faild\n",ERROR_VALUE);
            else if(numBytes == 0)
                Deal_User_Error("recv", "connection closed prematurely", ERROR_VALUE);
            totalBytesRcvd += numBytes;
        }

        return totalBytesRcvd;
    }   
}

void Clean_Zombies_Process(int *childProcCount)
{
    int processID = waitpid((pid_t) -1, NULL, WNOHANG);//Non=blocking wait
    if(processID < 0)
        Deal_System_Error("waitpid() faild", ERROR_VALUE);
    else if(processID > 0)
        (*childProcCount)--;
}

void TCP_Server_Runing(Client_Sock *ClntSock_Table)
{
    char *cmd;
    while(1)
    {
        system("date");
        printf("INPUT CMD: ");
        fgets(cmd,100,stdin);
        if(strcmp(cmd,"client") == 0)
        {
            Travel_Client_Table(ClntSock_Table, 1);
        }
        else
        {
            printf("wrong cmd\n");
        }
    }
}

int Insert_Client_Table(Client_Sock *ClntSock_Table, int sock)
{
    int i;
    for(i = 0; i < CLIENT_NUM; i++)
    {
        if(ClntSock_Table[i].sta == False)
        {
            ClntSock_Table[i].sock_id = sock;
            ClntSock_Table[i].sta = True;
            return i;
        }
    }
    printf("Client_Sock_Table filled\n");
    return -1;
}

void Travel_Client_Table(Client_Sock *ClntSock_Table, char cmd)
{
    int i;
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);

    for(i = 0; i < CLIENT_NUM; i++)
    {
        if(ClntSock_Table[i].sta == True)
        {
            if(getpeername(ClntSock_Table[i].sock_id, (struct sockaddr *)&localAddr,&addrSize) < 0)
                ClntSock_Table[i].sta = False;
            else
            {
                if(cmd == 1)
                    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
            }
        }
    }
}

void HandleTCPClient(int clntSocket)
{
    char buffer[BUFFSIZE];
    ssize_t numBytesRcved = 1;
    ssize_t numBytesSent;
    static FILE *tmp_fp;
    uint16_t ReadCount = 0;
    TransInfo vi;
    uint8_t outbuf[MAX_WIRE_SIZE];

    memset(&vi, 0, sizeof(vi));

    FILE *str = fdopen(clntSocket, "r+");
    if(str == NULL)
        Deal_System_Error("fdopen() faild", ERROR_VALUE);

    system("PATH=/bin:/sbin:/usr/bin:/usr/sbin");
    system("export PATH");

    if((tmp_fp = fopen(TMP_Path,"r+")) == NULL)
        Deal_User_Error("fopen() error", "temp file not exit",ERROR_VALUE);

    while(numBytesRcved > 0)
    {
        numBytesRcved = TCP_nReceive(clntSocket, buffer, 0);
        if(numBytesRcved != 0)
        {
            buffer[numBytesRcved - 1] = '\0';
            strcat(buffer,CMD_TMP_PATH);
            system(buffer);
            
            while(1)
            {
                vi.offset = ftell(tmp_fp);
                ReadCount = fread(vi.data,sizeof(uint8_t),BUFFER_SIZE,tmp_fp);
                printf("readcount=%d\n",ReadCount);
                if(ReadCount < BUFFER_SIZE)
                {
                    if(feof(tmp_fp))
                    {
                        vi.date_size = ReadCount;
                        Encode(&vi, outbuf, MAX_WIRE_SIZE);
                        PutMsg(outbuf,sizeof(TransInfo),str);
                        break;
                    }
                    else if(ferror(tmp_fp))
                        Deal_User_Error("fread()","error", ERROR_VALUE);
                }
                vi.date_size = ReadCount;
                Encode(&vi, outbuf, MAX_WIRE_SIZE);
                PutMsg(outbuf,sizeof(TransInfo),str);
            }
            fseek(tmp_fp, 0, SEEK_SET);
        }
    }
    close(clntSocket);
}