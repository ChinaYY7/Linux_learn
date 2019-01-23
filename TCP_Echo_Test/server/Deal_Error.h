#ifndef _DEAL_ERROR_H
#define _DEAL_ERROR_H
#include "apue.h"
#define Exit_value -1
void Deal_User_Error(const char *msg, const char *detail);
void Deal_System_Error(const char *msg);
#endif