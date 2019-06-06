#ifndef _S_SERVERUTILITY_H
#define _S_SERVERUTILITY_H
#include "apue.h"
#include <sys/socket.h>
#include <netdb.h>

#define CONFIG_PATH "./config/connect.config"
#define TMP_Path "./temp/temp"

void Get_Client_Config(char *server, char *service, int argc, char **argv);
#endif