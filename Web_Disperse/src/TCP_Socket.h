#ifndef _TCP_SOCKET_H
#define _TCP_SOCKET_H
#include <netdb.h>
#include "apue.h"

#define SERVER_LEN 100
#define SERVICE_LEN 100
#define MAXPENDING 5

int SetupTCPServerSocket(const char *service);
int SetupTCPClientSocket(const char *host, const char *service);
int AcceptTCPConnection(int servSock);
int Get_Address(int sock_fd, char *Buffer);
int Get_Address_Local(int sock_fd, char *Buffer);
void PrintSockAddress(const struct sockaddr *address, FILE *stream);
int Get_Sock_Name(int sock_fd);
int Get_Peer_Name(int sock_fd);
int TCP_nSend(int sock_fd, const void *buf, size_t buf_len);
ssize_t recv_tcp(int sock_fd, void *buf);
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len);
#endif
