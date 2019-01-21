#ifndef _TCPCLIENTUTILITY_H
#define _TCPCLIENTUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>
int SetupTCPClientSocket(const char *host, const char *service);

#endif