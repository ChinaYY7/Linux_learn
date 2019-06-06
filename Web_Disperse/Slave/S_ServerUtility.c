#include "S_ServerUtility.h"
#include "Deal_Error.h"
#include "TCP_Socket.h"

//获取客户端配置
void Get_Server_Config(char *server, char *service, int argc, char **argv)
{
    FILE *Config_file_fp;
    char addrBuffer[SERVICE_LEN];
    char *token;
    
    if(argc ==3)
    {
        strcpy(server, argv[1]);
        strcpy(service, argv[2]);
    }
    else if(argc == 1)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            User_Error_Exit("fopen()", "connect.config not exit!");

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);

        fclose(Config_file_fp);
    }
    else
        User_Error_Exit("wrong arguments","<Server Address> <Server Port>");
}

//回应浏览器的请求
void Reponse_Browser(int Browser_Sock, const char *Browser_Port)
{
    ssize_t numBytesSent;
    char buffer[50];
    char buffer_tmp[100];
    char addrbuffer[100];

    printf("Recevie A Browser's Request\n");
    memset(buffer, 0, sizeof(buffer));

    sprintf(buffer, "HTTP/1.0 200 OK\r\n");
    strcat(buffer, "Content-Type: text/html\r\n\r\n");

    Get_Address_Local(Browser_Sock,addrbuffer);
    sprintf(buffer_tmp,"Slave_Server(%s:%s) at your service......\r\n\r\n",addrbuffer,Browser_Port);    
    strcat(buffer, buffer_tmp);
    
    numBytesSent = TCP_nSend(Browser_Sock, buffer, strlen(buffer));

    close(Browser_Sock);
}