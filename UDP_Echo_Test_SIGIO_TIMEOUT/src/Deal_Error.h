#ifndef _DEAL_ERROR_H
#define _DEAL_ERROR_H
#include "apue.h"
#define ERROR_VALUE -1
void Deal_User_Error(const char *msg, const char *detail, int Exit_Value);
void Deal_System_Error(const char *msg, int Exit_Value);
#endif