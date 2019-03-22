#ifndef _FTP_CLIENTUTILITY_H
#define _FTP_CLIENTUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>

#define SERVER_LEN 100
#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "
#define TMP_Path "./temp/temp"

void Get_Server_Config(char *server, char *service, int argc, char **argv);
int SetupTCPClientSocket(const char *host, const char *service);
int TCP_nSend(int sock_fd, const void *buf, size_t buf_len);
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len);
int Get_Sock_Name(int sock_fd);
int Get_Peer_Name(int sock_fd);
#endif