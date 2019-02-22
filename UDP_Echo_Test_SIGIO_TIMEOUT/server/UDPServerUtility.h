#ifndef _UDPSERVERUTILITY_H
#define _UDPSERVERUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h"
#define MAXSTRLEN 4096

int SetupUDPServerSocket(const char *service);
int UDP_Recive_From(int servSock, void *buffer, struct sockaddr_storage *clntAddr, socklen_t *clntAddrLen);
int UDP_Send_To(int servSock, const void *buffer, size_t send_num, struct sockaddr_storage *clntAddr, socklen_t clntAddrLen);
void HandleUDPClient(int servSock);
#endif