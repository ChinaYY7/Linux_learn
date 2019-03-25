#include "Encode.h"
#include "Deal_Error.h"
#include <netinet/in.h>

size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize, uint16_t head)
{

    if(bufsize < sizeof(TransInfo))
        User_Error_Exit("Output buffer too small","");

    TransInfo *vm = (TransInfo *)outbuf;
    memset(outbuf, 0, sizeof(TransInfo));
    v->header= head;

    //printf("header=%x, offset=%d, date_size=%d\n",v->header,v->offset,v->date_size);

    if(head == FILE_HEAD)  //编码的是文件内容
        vm->offset = htonl(v->offset);

    vm->header = htons(v->header);
    vm->date_size = htons(v->date_size);

    for(int i = 0; i < v->date_size; i++)
        vm->data[i] = v->data[i];

    return sizeof(TransInfo);
}

Bool Decode(TransInfo *v, uint8_t *inbuf, const size_t msize)
{

    TransInfo *vm = (TransInfo *)inbuf;
    
    v->header = ntohs(vm->header);
    
    if(msize < sizeof(TransInfo))
        User_Error_Exit("INBUF","too small !");
    

    if(v->header == FILE_HEAD)    //解码文件
    {
        v->offset =  ntohl(vm->offset);
        v->date_size = ntohs(vm->date_size);
        for(int i = 0; i < v->date_size; i++)
            v->data[i] = vm->data[i];

        return True;
    }
    else if(v->header == MESSEGE_HEAD) //解码消息
    {
        v->date_size = ntohs(vm->date_size);
        for(int i = 0; i < v->date_size; i++)
            v->data[i] = vm->data[i];

        return True;
    }
    else
        return False;
}