#ifndef _S_SERVERUTILITY_H
#define _S_SERVERUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>

#define CONFIG_PATH "./config/connect.config"

void Get_Server_Config(char *server, char *service, int argc, char **argv);
void Reponse_Browser(int Browser_Sock, const char *Browser_Port);
#endif