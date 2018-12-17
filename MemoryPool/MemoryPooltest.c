
#include <stdio.h>
#include "MemoryPool.h"
#define LOOP 5
#define ALLOC_SIZE 8
int main(void)
{
	Memory_Pool *pool = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	int i = 0,cmd;
    pool = Memory_Pool_Init(1024);
	if (pool == NULL)
		printf("memory pool init failed\n");
	Get_Memory_Info(pool);
	for (i = 0; i < 2; i++) 
    {
		p1 = (char *)Memory_Malloc(pool, ALLOC_SIZE);
		if (p1 == NULL)
			printf("Malloc failed\n");
		else
			printf("Malloc success\n");
		Memory_Free(pool, p1);
        printf("\n");
	}
	p1 = (char *)Memory_Malloc(pool, 256);
	if (p1 == NULL)
		printf("Malloc failed\n");
	else
		printf("Malloc success\n");
	p2 = (char *)Memory_Malloc(pool, 512);
	if (p1 == NULL)
		printf("Malloc failed\n");
	else
		printf("Malloc success\n");
	Memory_Free(pool, p1);
	Memory_Free(pool, p2);
    while(1)
    {
        printf("\ninput 0 to close memory pool:");
        scanf("%d", &cmd);
        if(cmd == 0)
            Memory_Pool_Destroy(pool);
        else if(cmd == 1)
            break;
    }
	return 0;
}
