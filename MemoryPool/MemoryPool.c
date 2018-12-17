/*
++ Memory_Pool +++ Memory_Map_Table +++ Memory_Chunk_Table +++ Memory++
*/
#include "MemoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//内存池初始化
Memory_Pool_P Memory_Pool_Init(Size_V Need_Size)
{
	Memory_Pool_P Pool;
	Memory_Chunk_Table_P Chunk_Table_Tmp;
	Memory_Map_Table_P Map_Table_Tmp;
	Size_V Total_Size, Alloc_Size;
	Num_V Block_Num;
	int i = 0;

	if (Need_Size < 0 || Need_Size > MAX_POOL_SIZE) 
	{
		printf("Memory_Pool_Init(): Invalid size(%d)\n", Need_Size);
		return NULL;
	}

	Block_Num = ((Need_Size + BLOCK_SIZE - 1) / BLOCK_SIZE);//向上取整， 即不足一块按一块来算
	Alloc_Size = Block_Num * BLOCK_SIZE;
	Total_Size = sizeof(Memory_Pool) + (sizeof(Memory_Map_Table) + sizeof(Memory_Chunk_Table)) * Block_Num + Alloc_Size;

	Pool = (Memory_Pool_P)malloc(Total_Size);
	if (Pool == NULL) 
	{
		perror("Malloc failed\n");
		return NULL;
	}
	memset((char *)Pool, 0, Pool->Total_Size);          //分配的存储内容清空
	
	Pool->Block_Size = BLOCK_SIZE;
	Pool->Block_Num = Block_Num;
	Pool->Alloc_Memory_Size = Alloc_Size;
	Pool->Total_Size = Total_Size;
	Pool->Used_Size = 0;
	Pool->Chunk_Num = 0;
	Pool->Used_Block_Num = 0;
	
    Pool->Chunk_Table = (Memory_Chunk_Table_P)(Pool + sizeof(Memory_Pool) + sizeof(Memory_Map_Table) * Pool->Block_Num);
	Pool->Map_Table = (Memory_Map_Table_P)(Pool + sizeof(Memory_Pool));
	Pool->Memory_Start = (Point_V *)(Pool + sizeof(Memory_Pool) + (sizeof(Memory_Map_Table) + sizeof(Memory_Chunk_Table)) * Pool->Block_Num);
	for (i = 0; i < Pool->Block_Num; i++) 
	{
		Map_Table_Tmp = (Memory_Map_Table_P)(Pool->Map_Table + i * sizeof(Memory_Map_Table));
		Map_Table_Tmp->Index = i;
		Map_Table_Tmp->Block_P = Pool->Memory_Start + i * Pool->Block_Size;
		Map_Table_Tmp->Used = False;

		Chunk_Table_Tmp = (Memory_Chunk_Table_P)(Pool->Chunk_Table + i * sizeof(Memory_Chunk_Table));
		Chunk_Table_Tmp->Block_Num = 0;
		Chunk_Table_Tmp->Start_Block_Index = -1;
		Chunk_Table_Tmp->Start_Block_P = NULL;
		Chunk_Table_Tmp->Used = False;
	}
    //printf("Memory_Pool_Init: Total Size: %d, Block Num: %d, Block Size: %d\n", Pool->Alloc_Memory_Size, Pool->Block_Num, Pool->Block_Size);
    return Pool;
}
//内存池分配
void *Memory_Malloc(Memory_Pool_P Pool, Size_V Need_Size)
{
	Num_V Need_Block_Num = 0;
	Num_V Block_Num = 0;
	Num_V Start_Index = -1;
	Memory_Chunk_Table_P Chunk_Table_Tmp;
	Memory_Map_Table_P Map_Table_Tmp;
	
	int i = 0;

	if (Need_Size <= 0 || Need_Size > Pool->Alloc_Memory_Size) 
	{
		printf("Invalid size(%d)\n", Need_Size);
		return NULL;
	}
	if (Need_Size > Pool->Alloc_Memory_Size - Pool->Used_Size) 
	{
		printf("Free memory(%u) is less than allocate(%u)\n",Pool->Alloc_Memory_Size - Pool->Used_Size, Need_Size);
		return NULL;
	}
	Need_Block_Num = (Need_Size + Pool->Block_Size - 1) / Pool->Block_Size;

	for (i = 0; i < Pool->Block_Num; i++)
    {
		Map_Table_Tmp = (Memory_Map_Table_P)(Pool->Map_Table + i * sizeof(Memory_Map_Table));
		if(Map_Table_Tmp->Used == False)
		{
			if(Block_Num == 0)
				Start_Index = i;
			Block_Num++;
		}
		else
		{
			Start_Index = -1;
			Block_Num = 0;
		}
		
		if(Block_Num == Need_Block_Num)
			break;
	}
	if (Start_Index == -1) 
	{
		printf("No available memory to used\n");
		return NULL;
	}
	for (i = 0; i < Pool->Block_Num; i++) 
	{
		Chunk_Table_Tmp = (Memory_Chunk_Table_P)(Pool->Chunk_Table + i * sizeof(Memory_Chunk_Table));
		if (Chunk_Table_Tmp->Used == False) 
			break;
		Chunk_Table_Tmp = NULL;
	}
	if (Chunk_Table_Tmp == NULL) 
		return NULL;

	Map_Table_Tmp = (Memory_Map_Table_P)(Pool->Map_Table +  Start_Index * sizeof(Memory_Map_Table));
	Chunk_Table_Tmp->Start_Block_P = Map_Table_Tmp->Block_P;
	Chunk_Table_Tmp->Start_Block_Index = Map_Table_Tmp->Index;
	Chunk_Table_Tmp->Block_Num = Block_Num;
	Chunk_Table_Tmp->Used = True;
	
	for (i = Start_Index; i < Start_Index + Block_Num; i++)
	{
		Map_Table_Tmp = (Memory_Map_Table_P)(Pool->Map_Table +  i * sizeof(Memory_Map_Table));
		Map_Table_Tmp->Used= True;
	}
	printf("Alloc size: %d, Block Info: (start: %d, end: %d, cnt: %d)\n", Need_Block_Num * Pool->Block_Size, Start_Index, Start_Index + Block_Num - 1, Block_Num);
	Pool->Chunk_Num++;
	Pool->Used_Block_Num+=Block_Num;
	Pool->Used_Size += Block_Num * Pool->Block_Size;
	return Chunk_Table_Tmp->Start_Block_P;
}
//释放回内存池
void Memory_Free(Memory_Pool_P Pool, void *Memory_P)
{
	Memory_Chunk_Table_P Chunk_Table_Tmp;
	Memory_Map_Table_P Map_Table_Tmp;
	int i = 0;
	int block_start_index = 0;
	int block_cnt = 0;

	if (Memory_P == NULL) {
		printf("memory_free(): memory is NULL\n");
		return;
	}
	if (Pool == NULL) {
		printf("Pool is NULL\n");
		return;
	}

	for (i = 0; i < Pool->Block_Num; i++) 
	{
		Chunk_Table_Tmp = (Memory_Chunk_Table_P)(Pool->Chunk_Table + i * sizeof(Memory_Chunk_Table));
		if (Chunk_Table_Tmp->Start_Block_P == Memory_P) 
			break;
	}
    for (i = Chunk_Table_Tmp->Start_Block_Index; i < Chunk_Table_Tmp->Start_Block_Index + Chunk_Table_Tmp->Block_Num; i++) 
	{
		Map_Table_Tmp = (Memory_Map_Table_P)(Pool->Map_Table + i * sizeof(Memory_Map_Table));
		Map_Table_Tmp->Used = False;
	}
	Chunk_Table_Tmp->Used = False;
	Pool->Used_Size-=Chunk_Table_Tmp->Block_Num * Pool->Block_Size;
	Pool->Chunk_Num-=1;
	Pool->Used_Block_Num -= Chunk_Table_Tmp->Block_Num;
	printf("Block free: (start: %d, end: %d, cnt: %d)\n", Chunk_Table_Tmp->Start_Block_Index, Chunk_Table_Tmp->Start_Block_Index + Chunk_Table_Tmp->Block_Num - 1, Chunk_Table_Tmp->Block_Num);
}
//释放内存池分配的内存
void Memory_Pool_Destroy(Memory_Pool_P Pool)
{
	if (Pool == NULL) 
	{
		printf("memory_pool_destroy: pool is NULL\n");
		return;
	}
	free(Pool);
	Pool = NULL;
}

void Get_Memory_Info(Memory_Pool_P Pool)
{
	printf("\nMemory Pool Size:%u\n",Pool->Alloc_Memory_Size);
	printf("Memory Pool Used Size:%u\n",Pool->Used_Size);
	printf("Memory Pool Used Chunk Num:%u\n\n",Pool->Chunk_Num);
}