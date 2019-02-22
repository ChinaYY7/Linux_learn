#ifndef _ADDRESSUTILITY_
#define _ADDRESSUTILITY_
#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

void PrintSockAddress(const struct sockaddr *address, FILE *stream);
#endif