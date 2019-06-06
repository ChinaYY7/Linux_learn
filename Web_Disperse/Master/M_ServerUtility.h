#ifndef _M_SERVERUTILITY_H
#define _M_SERVERUTILITY_H
#include <netinet/in.h>
#include "apue.h"

#define CONFIG_PATH "./config/connect.config"
#define TMP_Path "./temp/temp"
#define CMD_TMP_PATH " 1>./temp/temp 2>./temp/temp"
#define CLIENT_NUM 10
#define TBL_SIZE 10

typedef struct Address_Tbl{

    char address[INET6_ADDRSTRLEN];
    Bool Enable;
    int Sock;
}Slave_Server_Address;

void HandleTCPClient(int clntSocket);
void Reponse_Chrom(int Chrom_Sock);
void Talk_Slave(int Slave_Sock);
int Slave_Server_Address_Empty(void);
int Select_Server_Address(void);
void Get_Server_Config(char *Chrom_Port, char *Slave_Port, int argc, char **argv);
void Clean_Zombies_Process(int *childProcCount);
#endif