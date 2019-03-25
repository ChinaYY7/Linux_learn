#include <netinet/in.h>
#include "Framer.h"
#include "Deal_Error.h"

//读取一帧信息
//若接收缓冲区有数据且可用读取成功返回读取的对象数
//若缓存区没有数据或者数据暂时不可用，阻塞，直到有数据
//若连接断开，将报错，并返回0
int GetNextMsg(FILE *in, uint8_t *buf, size_t buffsize)
{
    uint16_t msize = 0;
    uint16_t extra = 0;
    uint16_t fread_count;
    
    int rnt;  

    rnt = fread(&msize, sizeof(uint16_t), 1, in);
    if(rnt != 1)
    {
        if(errno != 0) 
        {
            System_Error("fread(msize)");
            return 0;
        }
    }
        
    msize = ntohs(msize);
    
    if(msize > buffsize)
    {
        extra = msize - buffsize;
        msize = buffsize;
    }

    if((fread_count = fread(buf, sizeof(uint8_t), msize, in)) != msize)
    {
        if(errno != 0)
        {
            System_Error("fread(buf)");
            return 0;
        }
    }
    
    if(extra > 0)
    {
        uint8_t waste[BUFFSIZE];
        if(fread(waste, sizeof(uint8_t), extra, in) != extra)
        {
            System_Error("fread(waste)");
            return 0;
        }
        return -(msize + extra);
    }
    else
        return fread_count + rnt;
}

//写一帧信息
//若发送缓冲区有空间且可用返回写入的对象数
//若缓存区空间不足或者数据暂时不可用，阻塞，直到有空间可用
//若连接断开，将报错，并返回0
int PutMsg(const uint8_t *buf, size_t msgsize, FILE *out)
{
    if(msgsize > __UINT16_MAX__)
        User_Error_Exit("bufsize too long", "should < __UINT16_MAX__");
    
    uint16_t payloadsize = htons(msgsize);
    
    if((fwrite(&payloadsize, sizeof(uint16_t), 1, out) != 1) || (fwrite(buf, sizeof(uint8_t), msgsize, out) != msgsize))
    {
        System_Error("fwrite(payloadsize or buf)");
        return 0;
    }
        
    
    fflush(out);

    return msgsize + 1;
}