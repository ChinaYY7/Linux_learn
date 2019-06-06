#include "Ftp_ClientUtility.h"
#include "Deal_Error.h"
#include "TCP_Socket.h"

//获取客户端配置
void Get_Client_Config(char *server, char *service, int argc, char **argv)
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