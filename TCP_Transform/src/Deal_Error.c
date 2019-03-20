#include "Deal_Error.h"

void System_Error(const char *msg)
{
    perror(msg);
}

void User_Error(const char *msg, const char *detail)
{
    fputs(msg, stderr);
    fputs(":", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
}

void User_Error_Exit_v(const char *msg, const char *detail, int Exit_Value)
{
    User_Error(msg, detail);
    exit(Exit_Value);
}

void System_Error_Exit_v(const char *msg, int Exit_Value)
{
    System_Error(msg);
    exit(Exit_Value);
}

void User_Error_Exit(const char *msg, const char *detail)
{
    User_Error_Exit_v(msg, detail, ERROR_VALUE);
}

void System_Error_Exit(const char *msg)
{
    System_Error_Exit_v(msg, ERROR_VALUE);
}