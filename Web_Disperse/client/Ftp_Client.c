#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "Ftp_ClientUtility.h"
#include "Trans_Protocol.h"

int main(int argc, char *argv[])
{
    char server[SERVER_LEN], service[SERVICE_LEN];
    
    //从命令行或者配置文件获取服务器地址和端口号
    Get_Client_Config(server, service, argc, argv);
    
    //创建客服端命令端连接套接字
    printf("\nClient->Server: ");
    int sock = SetupTCPClientSocket(server, service);
    if (sock < 0)
        User_Error_Exit("SetupTCPClientSocket(sock) faild!", "Unable to connect");

    //使用流包装套接字
    FILE *Cmd_Str = fdopen(sock, "r+");
    if(Cmd_Str == NULL)
        System_Error_Exit("fdopen() faild");

    //给服务器传递连接的端口
    char Date_Port[6];
    sprintf(Date_Port,"%d",getpid() % 5000 + 6000);
    printf("Date_Port = %s\n",Date_Port);
    Send_Messege(Cmd_Str, Date_Port);

    //创建客户端监听TCP套接字
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    int Server_Sock = SetupTCPServerSocket(Date_Port);  
    if(Server_Sock < 0)
        System_Error_Exit("SetupTCPServerSocket(Server_Sock) faild");

    printf("Server->Client: ");
    int Date_Sock = accept(Server_Sock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    Get_Peer_Name(Date_Sock);

    close(Server_Sock);

    //使用流包装套接字
    FILE *Date_Str = fdopen(Date_Sock, "r+");
    if(Date_Str == NULL)
        System_Error_Exit("fdopen(Server_Sock) faild");


    char Cmd_buffer[BUFFSIZE], Cmd[BUFFSIZE];
    char *token;
    char Parameter[3][SERVICE_LEN];
    char Messege[MAX_WIRE_SIZE];

    while(1)
    {   
        //输入并处理命令
        printf("\nftp@cmd:   ");
        fgets(Cmd_buffer, BUFFSIZE, stdin);
        size_t cmdStringLen = strlen(Cmd_buffer);
        Cmd_buffer[cmdStringLen - 1] = '\0'; //fgets会将输入的回车加入字符串，需要消除掉

        strcpy(Cmd, Cmd_buffer);

        Bool Cmd_sta = Deal_Cmd(Cmd_buffer, Parameter, 3);
        if(strcmp(Parameter[0],"down") == 0)
        {
            if(Cmd_sta == True)
            {
                Send_Messege(Cmd_Str, Cmd);
                int Recv_Bytes = Recv_File(Date_Str, Parameter[2]);

                printf("Download file : %s completed, Download %d bytes\n", Parameter[1], Recv_Bytes);
            }
            else
            {
                printf("Download Parameter is wrong : down <source> <target>\n");
            }
            //Recv_Messege(Cmd_Str, Messege);
            //printf("Messege from server: %s\n", Messege);
        }
        else
        {
            Send_Messege(Cmd_Str, Cmd);
            Recv_File(Cmd_Str, TMP_Path);
            system("cat ./temp/temp");
        }
    }
    fclose(Cmd_Str);
    fclose(Date_Str);
    return 0;
} 