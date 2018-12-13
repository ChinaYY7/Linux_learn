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
    pool = memory_pool_init(1024 * 1024 * 100, 512);
	if (pool == NULL)
		printf("memory pool init failed\n");
	for (i = 0; i < 2; i++) 
    {
		p1 = (char *)Memory_malloc(pool, ALLOC_SIZE);
		if (p1 == NULL)
			printf("Malloc failed\n");
		else
			printf("Malloc success\n");
		memory_free(pool, p1);
        printf("\n");
	}
	p1 = (char *)Memory_malloc(pool, 256);
	if (p1 == NULL)
		printf("Malloc failed\n");
	else
		printf("Malloc success\n");
	p2 = (char *)Memory_malloc(pool, 512);
	if (p1 == NULL)
		printf("Malloc failed\n");
	else
		printf("Malloc success\n");
	memory_free(pool, p1);
	memory_free(pool, p2);
    while(1)
    {
        printf("\ninput 0 to close memory pool:");
        scanf("%d", &cmd);
        if(cmd == 0)
            memory_pool_destroy(pool);
        else if(cmd == 1)
            break;
    }
	return 0;
}
