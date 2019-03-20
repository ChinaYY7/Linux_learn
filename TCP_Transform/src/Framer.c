#include "Framer.h"
#include "Deal_Error.h"
#include <netinet/in.h>
int GetNextMsg(FILE *in, uint8_t *buf, size_t buffsize)
{
    uint16_t msize = 0;
    uint16_t extra = 0;
    
    int rnt;  

    rnt = fread(&msize, sizeof(uint16_t), 1, in);
    if(rnt != 1)
    {
        if(errno != 0) 
            User_Error_Exit("fread(msize)","read unexpected size !");
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
        User_Error_Exit("fread(buf)","read unexpected size !");

    if(extra > 0)
    {
        uint8_t waste[BUFFSIZE];
        if(fread(waste, sizeof(uint8_t), extra, in) != extra)
            User_Error_Exit("fread(waste)","read unexpected size !");
        return -(msize + extra);
    }
    else
        return msize;
}

int PutMsg(const uint8_t *buf, size_t msgsize, FILE *out)
{
    if(msgsize > __UINT16_MAX__)
        User_Error_Exit("bufsize too long", "should < __UINT16_MAX__");
    
    uint16_t payloadsize = htons(msgsize);
    
    if((fwrite(&payloadsize, sizeof(uint16_t), 1, out) != 1) || (fwrite(buf, sizeof(uint8_t), msgsize, out) != msgsize))
        User_Error_Exit("fwrite(payloadsize or buf)", "write unexpected size !");
    
    fflush(out);

    return msgsize;
}