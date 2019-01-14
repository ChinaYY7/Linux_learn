#include "apue.h"
#include <stdio.h>
#include "MemoryPool.h"
#define LOOP 5
#define ALLOC_SIZE 8
int main(void)
{
	Memory_Pool *pool = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
	char *charp1 = NULL;
	int *intp2 = NULL;
	int i = 0,cmd;
    pool = Memory_Pool_Init(1024);
	if (pool == NULL)
		printf("memory pool init failed\n");
	Get_Memory_Info(pool);
	charp1 = (char *)Memory_Malloc(pool, sizeof(char) * 100);
	strcpy(charp1,"This is a test");
	printf("Malloc char success and write a string: %s\n", charp1);
	
	intp2 = (int *)Memory_Malloc(pool, sizeof(int) * 10);
	for(i = 0; i < 10; i++)
	{
		intp2[i] = 10000000-i;
		printf("%d ",intp2[i]);
	}
	printf("\n");
	Get_Memory_Info(pool);
	Memory_Free(pool, (void *)&intp2);
	Memory_Free(pool, (void *)&charp1);
	/*
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
	*/
	return 0;
}
