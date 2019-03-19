#include "Encode.h"
#include "Deal_Error.h"
#include <netinet/in.h>

size_t Encode(TransInfo *v, uint8_t *outbuf, size_t bufsize)
{
    printf("Encode: Bin\n");

    if(bufsize < sizeof(TransInfo))
        Deal_User_Error("Output buffer too small","",ERROR_VALUE);

    TransInfo *vm = (TransInfo *)outbuf;
    memset(outbuf, 0, sizeof(TransInfo));
    v->header= HEAD;

    printf("header=%x, offset=%d, date_size=%d\n",v->header,v->offset,v->date_size);

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
    printf("Decode: Bin\n");

    TransInfo *vm = (TransInfo *)inbuf;
    
    v->header = ntohs(vm->header);
    
    if(msize < sizeof(TransInfo))
        Deal_User_Error("INBUF","too small !", False);
    
    if(v->header != HEAD)
        Deal_User_Error("HEAD","Not Matching !", False);

    v->offset =  ntohl(vm->offset);
    v->date_size = ntohs(vm->date_size);
    for(int i = 0; i < v->date_size; i++)
        v->data[i] = vm->data[i];
    
    return True;
}