#ifndef _TRANS_PROTOCOL_H
#define _TRANS_PROTOCOL_H
#include "apue.h"

#define BUFFER_SIZE 4096
#define MAX_WIRE_SIZE 8192
#define HEAD 0X5400

typedef struct TransInfo
{
    uint32_t offset;
    uint16_t header;
    uint16_t date_size;
    uint8_t data[BUFFER_SIZE];
}TransInfo;

#endif