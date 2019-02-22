#include "Deal_Error.h"
void Deal_User_Error(const char *msg, const char *detail, int Exit_Value)
{
    fputs(msg, stderr);
    fputs(":", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(Exit_Value);
}

void Deal_System_Error(const char *msg, int Exit_Value)
{
    perror(msg);
    exit(Exit_Value);
}