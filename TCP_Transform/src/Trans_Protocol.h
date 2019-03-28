#ifndef _TRANS_PROTOCOL_H
#define _TRANS_PROTOCOL_H
#include "apue.h"

#define BUFFER_SIZE 4096
#define MAX_WIRE_SIZE 8192
#define FILE_HEAD 0X5400
#define MESSEGE_HEAD 0X5500

#define DELIM " "

typedef struct TransInfo
{
    uint32_t offset;
    uint16_t header;
    uint16_t date_size;
    uint8_t data[BUFFER_SIZE];
}TransInfo;

size_t Compute_TransInfo_Size(TransInfo *v);
size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize, uint16_t head);
Bool Decode(TransInfo *v, uint8_t *inbuf, const size_t msize);
int GetNextMsg(FILE *in, uint8_t *buf, size_t buffsize);
int PutMsg(const uint8_t *buf, size_t msgsize, FILE *out);

int Trans_Send(FILE *out, TransInfo *v, uint16_t head);
int Trans_Recv(FILE *in, TransInfo *v);
int Send_Messege (FILE *out, const char *string);
int Recv_Messege(FILE *out, char *string);
uint64_t Send_File(FILE *out, const char *file_path);
uint64_t Recv_File(FILE *in, const char *file_path);

Bool Deal_Cmd(char *Cmd_Buffer, char Parameter[][100], int Parameter_Num);
#endif