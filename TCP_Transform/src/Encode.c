#include "Encode.h"
#include "Deal_Error.h"
#include <netinet/in.h>

size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize)
{

    if(bufsize < sizeof(TransInfo))
        User_Error_Exit("Output buffer too small","");

    TransInfo *vm = (TransInfo *)outbuf;
    memset(outbuf, 0, sizeof(TransInfo));
    v->header= HEAD;

    //printf("header=%x, offset=%d, date_size=%d\n",v->header,v->offset,v->date_size);

    vm->header = v->header;
    vm->header = htons(v->header);
    vm->offset = htonl(v->offset);
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
    
    if(v->header != HEAD)
        User_Error_Exit("HEAD","Not Matching !");

    v->offset =  ntohl(vm->offset);
    v->date_size = ntohs(vm->date_size);
    for(int i = 0; i < v->date_size; i++)
        v->data[i] = vm->data[i];
    
    return True;
}