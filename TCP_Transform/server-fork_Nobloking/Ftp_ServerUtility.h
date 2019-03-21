#ifndef _FTP_SERVERUTILITY_H
#define _FTP_SERVERUTILITY_H
#include "apue.h"
static const int MAXPENDING = 5;

#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

#define CLIENT_NUM 10
#define CMD_TMP_PATH " 1>/mnt/f/Linux_code/Linux_learn/TCP_Transform/server-fork_Nobloking/temp/temp 2>/mnt/f/Linux_code/Linux_learn/TCP_Transform/server-fork_Nobloking/temp/temp"
#define TMP_Path "/mnt/f/Linux_code/Linux_learn/TCP_Transform/server-fork_Nobloking/temp/temp"


void HandleTCPClient(int clntSocket);
void Get_Server_Config(char *service, int argc, char **argv);
int SetupTCPServerSocket(const char *service);
int AcceptTCPConnection(int servSock);
int Get_Sock_Name(int sock_fd);
int Get_Peer_Name(int sock_fd);
void Clean_Zombies_Process(int *childProcCount);
#endif