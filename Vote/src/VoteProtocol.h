#ifndef _VOTEPROTOCOL_H
#define _VOTEPROTOCOL_H
#include "apue.h"

#define MAX_CANDIDATE 1000
#define MAX_WIRE_SIZE 500

typedef struct VoteInfo
{
    uint64_t count;
    int candidate;
    Bool isInquiry;
    Bool isResponse;
}VoteInfo;

#endif