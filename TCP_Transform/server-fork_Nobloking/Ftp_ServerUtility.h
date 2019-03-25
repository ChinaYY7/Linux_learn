#ifndef _FTP_SERVERUTILITY_H
#define _FTP_SERVERUTILITY_H
#include "apue.h"

#define CONFIG_PATH "./config/connect.config"
#define TMP_Path "./temp/temp"
#define CMD_TMP_PATH " 1>./temp/temp 2>./temp/temp"
#define CLIENT_NUM 10

void HandleTCPClient(int clntSocket);
void Get_Server_Config(char *service, int argc, char **argv);
void Clean_Zombies_Process(int *childProcCount);
#endif