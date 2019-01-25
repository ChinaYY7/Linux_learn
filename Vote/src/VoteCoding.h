#ifndef _VOTECODDING_H
#define _VOTECODEING_H

#include "apue.h"
#include "VoteProtocol.h"
#define VOTESTR "V"
#define INQSTR "I"
#define RESPONSESTR "R"

#define Decode Decode_Text
#define Encode Encode_Text

size_t Encode_Text(const VoteInfo *v, uint8_t *outbuf, const size_t bufsize);
Bool Decode_Text(VoteInfo *v, uint8_t *inbuf, const size_t msize);
size_t Encode_Bin(VoteInfo *v, uint8_t *outbuf, size_t bufsize);
Bool Decode_Bin(VoteInfo *v, uint8_t *inbuf, const size_t msize);

#endif