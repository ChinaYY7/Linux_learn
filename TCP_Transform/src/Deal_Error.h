#ifndef _DEAL_ERROR_H
#define _DEAL_ERROR_H
#include "apue.h"
#define ERROR_VALUE -1
void System_Error(const char *msg);
void User_Error(const char *msg, const char *detail);
void User_Error_Exit_v(const char *msg, const char *detail, int Exit_Value);
void System_Error_Exit_v(const char *msg, int Exit_Value);
void User_Error_Exit(const char *msg, const char *detail);
void System_Error_Exit(const char *msg);
#endif