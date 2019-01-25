#include "VoteCoding.h"
#include "Deal_Error.h"
#include <netinet/in.h>

static const char *MAGIC = "Voting";
#define DELIMSTR " "
#define BASE 10

size_t Encode_Text(const VoteInfo *v, uint8_t *outbuf, const size_t bufsize)
{
    uint8_t *bufptr = outbuf;
    long size = (size_t) bufsize;

    printf("Encode: Text\n"); 

    int rv = snprintf((char *)bufptr, size, "%s %c %s %d ", MAGIC, (v->isInquiry ? 'I' : 'V'), (v->isResponse ? "R" : ""), v->candidate);

    bufptr += rv;
    size -= rv;

    if(v->isResponse)
    {
        rv = snprintf((char *)bufptr, size, "%lu", v->count);
        bufptr += rv;
    }

    return (size_t) (bufptr - outbuf);
}

Bool Decode_Text(VoteInfo *v, uint8_t *inbuf, const size_t msize)
{
    char *tokon;

    tokon = strtok((char *)inbuf, DELIMSTR);

    printf("Decode: Text\n");

    if(tokon == NULL || strcmp(tokon, MAGIC) != 0)
        Deal_User_Error("MAGIC","Not Matching !", (int)False);
    
    tokon = strtok(NULL, DELIMSTR);
    if(strcmp(tokon, VOTESTR) == 0)
        v->isInquiry = False;
    else if(strcmp(tokon,INQSTR) == 0)
        v->isInquiry = True;
    else
        Deal_User_Error("Demand","Not Matching !", (int)False);
    
    tokon = strtok(NULL, DELIMSTR);
    if(tokon == NULL)
        Deal_User_Error("RESPONSESTR","Not Matching !", (int)False);

    if(strcmp(tokon, RESPONSESTR) == 0)
    {
        v->isResponse = True;
        tokon = strtok(NULL, DELIMSTR); //Get candidate ID
        if(tokon == NULL)
            Deal_User_Error("Candidate","Not Matching !", (int)False);
    }
    else
        v->isResponse = False;

    v->candidate = atoi(tokon);

    if(v->isResponse)
    {
        tokon = strtok(NULL, DELIMSTR);
        if(tokon == NULL)
            Deal_User_Error("Count","Not Matching !", (int)False);

        v->count = strtoll(tokon, NULL, BASE);
    }
    else
        v->count = 0L;

    return True;
}


/*
0  1  2  3  4  5  6    7     0  1  2  3  4  5  6  7  
|       Magic     |flag|     |        ZERO        |
|                  Candidate ID                   |
|                  Vote_cout_High                 |
|                  Vote_cout_Low                  |
*/

enum
{
    REQUEST_SIZE = 4,
    RESPONSE_SIZE = 12,
    COUNT_SHIFT = 32,
    INQUIRE_FLAG = 0x0100,
    RESPONS_FLAG = 0x0200,
    MAGIC_BIN = 0x5400,
    MAGIC_MASK = 0xfc00
};

typedef struct VoteMsgBin
{
    uint16_t header;
    uint16_t candidateID;
    uint32_t countHigh;
    uint32_t countLow;
}VoteMsgBin;

size_t Encode_Bin(VoteInfo *v, uint8_t *outbuf, size_t bufsize)
{
    printf("Encode: Bin\n");

    if((v->isResponse && bufsize < sizeof(VoteMsgBin)) || bufsize < 2 * sizeof(uint16_t))
        Deal_User_Error("Output buffer too small","",ERROR_VALUE);
    
    VoteMsgBin *vm = (VoteMsgBin *)outbuf;
    memset(outbuf, 0, sizeof(VoteMsgBin));

    vm->header = MAGIC_BIN;

    if(v->isInquiry)
        vm->header |= INQUIRE_FLAG;
    
    if(v->isResponse)
        vm->header |= RESPONS_FLAG;
    
    vm->header = htons(vm->header);
    vm->candidateID = htons(v->candidate);

    if(v->isResponse)
    {
        vm->countHigh = htonl(v->count >> COUNT_SHIFT);
        vm->countLow = htonl((uint32_t)v->count);
        return RESPONSE_SIZE;
    }
    else
        return REQUEST_SIZE;
}

Bool Decode_Bin(VoteInfo *v, uint8_t *inbuf, const size_t msize)
{
    printf("Decode: Bin\n");

    VoteMsgBin *vm = (VoteMsgBin *)inbuf;
    
    uint16_t header = ntohs(vm->header);
    
    if(msize < REQUEST_SIZE)
        Deal_User_Error("INBUF","too small !", False);
    if((header & MAGIC_MASK) != MAGIC_BIN)
        Deal_User_Error("MAGIC","Not Matching !", False);

    v->isResponse = ((header & RESPONS_FLAG) != 0);
    v->isInquiry = ((header & INQUIRE_FLAG) != 0);
    v->candidate = ntohs(vm->candidateID);

    if(v->isResponse && msize >= RESPONSE_SIZE)
        v->count = ((uint64_t)ntohl((uint64_t)vm->countHigh << COUNT_SHIFT) | (uint64_t) ntohl(vm->countLow));
    
    return True;
}