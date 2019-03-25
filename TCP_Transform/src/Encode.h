#ifndef _ENCODE_H
#define _ENCODE_H
#include "apue.h"
#include "Trans_Protocol.h"

size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize, uint16_t head);
Bool Decode(TransInfo *v, uint8_t *inbuf, const size_t msize);

#endif