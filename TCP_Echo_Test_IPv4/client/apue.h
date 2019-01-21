#ifndef _APUE_H
#define _APUE_H

#define Termux 0

#include <sys/types.h>
#include <sys/stat.h>
#if Termux == 1
#include <termios.h>
#else
#include <sys/termios.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DIR_MODE (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

#define BUFFIZE 4096


typedef enum BoolType{
    False = 0,
    True = 1
}Bool;


#endif
