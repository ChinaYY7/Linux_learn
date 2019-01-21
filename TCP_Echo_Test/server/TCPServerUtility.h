#ifndef _TCPSERVERUTILITY_H
#define _TCPSERVERUTILITY_H
#include "apue.h"
static const int MAXPENDING = 5;

void HandleTCPClient(int clntSocket);
int SetupTCPServerSocket(const char *service);
int AcceptTCPConnection(int servSock);
#endif