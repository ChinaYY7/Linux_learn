#ifndef _FRAMER_H
#define _FRAMER_H

#include "apue.h"

#define GetNextMsg GetNextMsg_Length
#define PutMsg PutMsg_Length

int GetNextMsg_Delim(FILE *in, uint8_t *buf, size_t buffsize);
int PutMsg_Delim(const uint8_t *buf, size_t msgsize, FILE *out);
int GetNextMsg_Length(FILE *in, uint8_t *buf, size_t buffsize);
int PutMsg_Length(const uint8_t *buf, size_t msgsize, FILE *out);
#endif