#ifndef _TCPSERVERUTILITY_H
#define _TCPSERVERUTILITY_H
#include "apue.h"
static const int MAXPENDING = 5;

#define CLIENT_NUM 10

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