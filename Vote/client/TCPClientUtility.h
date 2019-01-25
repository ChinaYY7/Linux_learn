#ifndef _TCPCLIENTUTILITY_H
#define _TCPCLIENTUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>

int SetupTCPClientSocket(const char *host, const char *service);
int TCP_nSend(int sock_fd, const void *buf, size_t buf_len);
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len);
int Get_Sock_Name(int sock_fd);
#endif