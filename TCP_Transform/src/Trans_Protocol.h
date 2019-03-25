#ifndef _TRANS_PROTOCOL_H
#define _TRANS_PROTOCOL_H
#include "apue.h"

#define BUFFER_SIZE 4096
#define MAX_WIRE_SIZE 8192
#define FILE_HEAD 0X5400
#define MESSEGE_HEAD 0X5500

typedef struct TransInfo
{
    uint32_t offset;
    uint16_t header;
    uint16_t date_size;
    uint8_t data[BUFFER_SIZE];
}TransInfo;

int Trans_Send(FILE *out, TransInfo *v, uint16_t head);
int Trans_Recv(FILE *in, TransInfo *v);
int Send_Messege (FILE *out, const char *string);
int Recv_Messege(FILE *out, char *string);
uint64_t Send_File(FILE *out, const char *file_path);
uint64_t Recv_File(FILE *in, const char *file_path);
#endif