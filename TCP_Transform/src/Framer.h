#ifndef _FRAMER_H
#define _FRAMER_H

#include "apue.h"
int GetNextMsg(FILE *in, uint8_t *buf, size_t buffsize);
int PutMsg(const uint8_t *buf, size_t msgsize, FILE *out);
#endif