#ifndef _UDPCLIENTUTILITY_H
#define _UDPCLIENTUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>
#define MAXSTRLEN 4096
int SetupUDPClientSocket(const char *host, const char *service, struct addrinfo **severAddr);
int SockAddrsEqual(struct sockaddr *Servaddr, struct sockaddr *Fromaddr);

#endif