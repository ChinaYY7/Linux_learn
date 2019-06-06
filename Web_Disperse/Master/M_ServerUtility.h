#ifndef _M_SERVERUTILITY_H
#define _M_SERVERUTILITY_H
#include <netinet/in.h>
#include "apue.h"

#define CONFIG_PATH "./config/connect.config"
#define TBL_SIZE 10

typedef struct Address_Tbl{

    char address[INET6_ADDRSTRLEN];
    Bool Enable;
    int Sock;
    int load;
}Slave_Server_Address;

void Reponse_Browser(int Browser_Sock);
int Talk_Slave(int Slave_Sock);
void Init_Slave_Server_Address_Tbl(void);
int Slave_Server_Address_Empty(void);
int Select_Server_Address(void);
void Get_Server_Config(char *Chrom_Port, char *Slave_Port, int argc, char **argv);
int Shield_SIGPIPE(void);
void Clean_Zombies_Process(int *childProcCount);
#endif