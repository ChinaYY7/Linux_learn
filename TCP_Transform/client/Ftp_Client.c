#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "Deal_Error.h"
#include "Ftp_ClientUtility.h"
#include "Trans_Protocol.h"

int main(int argc, char *argv[])
{
    char server[SERVER_LEN], service[SERVICE_LEN];
    
    //从命令行或者配置文件获取服务器地址和端口号
    Get_Server_Config(server, service, argc, argv);
    
    //创建客服端命令端连接套接字
    int sock = SetupTCPClientSocket(server, service);
    if (sock < 0)
        User_Error_Exit("SetupTCPClientSocket() faild!", "Unable to connect");

    //使用流包装套接字
    FILE *str = fdopen(sock, "r+");
    if(str == NULL)
        System_Error_Exit("fdopen() faild");

    char Cmd_buffer[BUFFSIZE];
    char *token;
    char Cmd[SERVICE_LEN],Parameter[3][SERVICE_LEN];
    char Messege[MAX_WIRE_SIZE];

    while(1)
    {
        //发送命令
        printf("\nftp@cmd:   ");
        fgets(Cmd_buffer, BUFFSIZE, stdin);
        size_t cmdStringLen = strlen(Cmd_buffer);
        Cmd_buffer[cmdStringLen - 1] = '\0';
        Send_Messege(str, Cmd_buffer);

        token = strtok(Cmd_buffer, DELIM);
        strcpy(Cmd,token);

        if(strcmp(Cmd,"down") == 0)
        {
            Recv_Messege(str, Messege);
            printf("Messege from server: %s\n", Messege);
        }
        else
        {
            Recv_File(str, TMP_Path);
            system("cat ./temp/temp");
        }
    }
    fclose(str);
    return 0;
} 