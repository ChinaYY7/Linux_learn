#ifndef _FTP_SERVERUTILITY_H
#define _FTP_SERVERUTILITY_H
#include "apue.h"
static const int MAXPENDING = 5;

#define CLIENT_NUM 10
#define CMD_TMP_PATH " > /mnt/f/Linux_code/Linux_learn/TCP_Transform/server-fork_Nobloking/temp"
#define TMP_Path "/mnt/f/Linux_code/Linux_learn/TCP_Transform/server-fork_Nobloking/temp"

typedef struct Free_Clnt_Sock{
    int sock_id;
    Bool sta;
}Client_Sock;

void HandleTCPClient(int clntSocket);
int SetupTCPServerSocket(const char *service);
int AcceptTCPConnection(int servSock);
int Get_Sock_Name(int sock_fd);
int Get_Peer_Name(int sock_fd);
void Clean_Zombies_Process(int *childProcCount);
void TCP_Server_Runing(Client_Sock *ClntSock_Table);
int Insert_Client_Table(Client_Sock *ClntSock_Table, int sock);
void Travel_Client_Table(Client_Sock *ClntSock_Table, char cmd);
#endif