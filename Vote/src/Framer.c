#include "Framer.h"
#include "Deal_Error.h"
#include <netinet/in.h>

static const char DELIMTER = '\n';

int GetNextMsg_Delim(FILE *in, uint8_t *buf, size_t buffsize)
{
    int count = 0;
    int nextchar;

    printf("Get: Delim\n");

    while(count < buffsize)
    {
        nextchar = getc(in);
        if(nextchar == EOF)
        {
            if(count > 0)
                Deal_User_Error("GetNextMsg()","Stream ended prematurely !", ERROR_VALUE);
            else
                return ERROR_VALUE;
        }
        if(nextchar == DELIMTER)
            break;
        
        buf[count++] = nextchar;
    }
    buf[count] = '\0';
    if(nextchar != DELIMTER)
        return -count;
    else
        return count;
}

int PutMsg_Delim(const uint8_t *buf, size_t msgsize, FILE *out)
{
    int i;

    printf("Put: Delim\n");

    for (i = 0; i < msgsize; i++)
    {
        if(buf[i] == DELIMTER)
            return ERROR_VALUE;
    }

    if(fwrite(buf, sizeof(uint8_t), msgsize, out) != msgsize)
        return ERROR_VALUE;
    fputc(DELIMTER,out);


    fflush(out);

    return msgsize;
}

int GetNextMsg_Length(FILE *in, uint8_t *buf, size_t buffsize)
{
    uint16_t msize = 0;
    uint16_t extra = 0;
    
    int rnt;  

    printf("Get: Length\n");  

    rnt = fread(&msize, sizeof(uint16_t), 1, in);
    if(rnt != 1)
    {
        if(errno != 0) 
            Deal_User_Error("fread(msize)","read unexpected size !", ERROR_VALUE);
        else
            return 0;
    }
        

    msize = ntohs(msize);
    
    if(msize > buffsize)
    {
        extra = msize - buffsize;
        msize = buffsize;
    }

    if(fread(buf, sizeof(uint8_t), msize, in) != msize)
        Deal_User_Error("fread(buf)","read unexpected size !", ERROR_VALUE);

    buf[msize] = '\0';

    if(extra > 0)
    {
        uint8_t waste[BUFFSIZE];
        if(fread(waste, sizeof(uint8_t), extra, in) != extra)
            Deal_User_Error("fread(waste)","read unexpected size !", ERROR_VALUE);
        return -(msize + extra);
    }
    else
        return msize;
}

int PutMsg_Length(const uint8_t *buf, size_t msgsize, FILE *out)
{
    printf("Put: Length\n"); 

    if(msgsize > __UINT16_MAX__)
        Deal_User_Error("bufsize too long", "should < __UINT16_MAX__", ERROR_VALUE);
    
    uint16_t payloadsize = htons(msgsize);
    
    if((fwrite(&payloadsize, sizeof(uint16_t), 1, out) != 1) || (fwrite(buf, sizeof(uint8_t), msgsize, out) != msgsize))
        Deal_User_Error("fwrite(payloadsize or buf)", "write unexpected size !", ERROR_VALUE);
    
    fflush(out);

    return msgsize;
}

