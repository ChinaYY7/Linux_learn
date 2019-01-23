#include "Deal_Error.h"
void Deal_User_Error(const char *msg, const char *detail)
{
    fputs(msg, stderr);
    fputs(":", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(Exit_value);
}

void Deal_System_Error(const char *msg)
{
    perror(msg);
    exit(Exit_value);
}