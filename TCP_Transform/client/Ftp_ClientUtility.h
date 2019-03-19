#ifndef _FTP_CLIENTUTILITY_H
#define _FTP_CLIENTUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>

#define TMP_Path "/mnt/f/Linux_code/Linux_learn/TCP_Transform/client/temp"

int SetupTCPClientSocket(const char *host, const char *service);
int TCP_nSend(int sock_fd, const void *buf, size_t buf_len);
int TCP_nReceive(int sock_fd, void *buf, size_t buf_len);
int Get_Sock_Name(int sock_fd);
int TCP_get_string(int sock_fd, char *str);
ssize_t TCP_get_all(int sock_fd);
#endif