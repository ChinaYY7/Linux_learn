/*
++ Memory_Pool +++ Memory_Map_Table +++ Memory_Alloc_Table +++ Memory++
*/
#include "MemoryPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//获取内存映射表的位置
Memory_Map_Table *map_table_pos(Memory_Pool *pool)
{
    Memory_Map_Table *p = (Memory_Map_Table *)(pool->memory_start + sizeof(Memory_Pool));
    return p;
}

//获取内存分配表的位置
Memory_Alloc_Table *alloc_talbe_pos(Memory_Pool *pool)
{
	Memory_Alloc_Table *p = (Memory_Alloc_Table *)(pool->memory_start + sizeof(Memory_Pool) + sizeof(Memory_Map_Table) * (pool->block_cnt));
    return p;
}

//获得memory在位置
char *memory_pos(Memory_Pool *pool)
{
    char *p = (char *)(pool->memory_start + sizeof(Memory_Pool) + (sizeof(Memory_Map_Table) + sizeof(Memory_Alloc_Table))* pool->block_cnt);
    return p;
}

Memory_Pool *memory_pool_init(int size, int increment)
{
	char *p = NULL;
	char *p_memory = NULL;
	Memory_Pool *pool = NULL;
	Memory_Alloc_Table *alloc_table = NULL;
	Memory_Alloc_Table *p_alloc_table = NULL;
	Memory_Map_Table *map_table = NULL;	
	Memory_Map_Table *p_map_table = NULL;
	unsigned int block_cnt = 0;
	unsigned int all_size = 0;
	int i = 0;

	if (size < 0 || size > MAX_POOL_SIZE) {
		printf("memory_pool_init(): Invalid size(%d)\n", size);
		return pool;
	}
	block_cnt = ((size + BLOCK_SIZE - 1) / BLOCK_SIZE); //向上取整， 即不足一块按一块来算
	all_size = sizeof(Memory_Pool) + (sizeof(Memory_Map_Table) + sizeof(Memory_Alloc_Table)) * block_cnt + size;
    p = (char *)malloc(all_size);
    if (p == NULL) {
		perror("Malloc failed\n");
		return pool;
	}
    memset(p, 0, all_size);          //分配的存储内容清空
	pool = (Memory_Pool *)p;
	pool->block_cnt = block_cnt;//内存池有多少块
	pool->block_size = BLOCK_SIZE;//块大小
	pool->increment = increment;//没用
	pool->internal_total_size = BLOCK_SIZE * block_cnt;//没用
	pool->total_size = size;//内存池大小
	pool->used_size = 0; //使用了多少容量
	pool->alloc_cnt = 0; //分配了多少块
	pool->memory_start = p;//内存池的首地址
    p_memory = memory_pos(pool);//用于分配的内存的起始地址
    map_table = map_table_pos(pool);//映射表内存首地址
	for (i = 0; i < block_cnt; i++) {
		p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
		p_map_table->index = i;
		p_map_table->p_block = p_memory + i * BLOCK_SIZE;
		p_map_table->used = 0;
	}
	alloc_table = alloc_talbe_pos(pool);//分配表首地址
	for (i = 0; i < block_cnt; i++) {
		p_alloc_table = (Memory_Alloc_Table *)((char *)alloc_table + i * sizeof(Memory_Alloc_Table));
		p_alloc_table->block_cnt = 0;
		p_alloc_table->block_start_index = -1;
		p_alloc_table->p_start = NULL;
		p_alloc_table->used = 0;
	}
    printf("memory_pool_init: total size: %d, block cnt: %d, block size: %d\n", pool->total_size, pool->block_cnt, BLOCK_SIZE);
    return pool;
}
void *Memory_malloc(Memory_Pool *pool, int size)
{
    char *p_start = NULL;
	int need_block_cnt = 0;
	Memory_Map_Table *map_table = NULL;
	Memory_Map_Table *p_map_table = NULL;
	Memory_Alloc_Table *alloc_table = NULL;
	Memory_Alloc_Table *p_alloc_table = NULL;
	int block_cnt = 0;
	int start_index = -1;
	int i = 0;

	if (size <= 0) {
		printf("Invalid size(%d)\n", size);
		return p_start;
	}
	if (size > pool->total_size) {
    	printf("%d is more than total size\n", size);
		return p_start;
	}
	if (size > pool->total_size - pool->used_size) {
		printf("Free memory(%d) is less than allocate(%d)\n",
		pool->total_size - pool->used_size, size);
		return NULL;
	}
	need_block_cnt = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	map_table = map_table_pos(pool);
	start_index = -1;
	for (i = 0; i < pool->block_cnt; i++)
    {
		p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
		if (p_map_table->used) {
			//printf("before alloc: map index: %d is used\n", i);
			block_cnt = 0;
			start_index = -1;
			continue;
		}
		if (start_index == -1) {
			start_index = i;
			//printf("start_index: %d\n", start_index);
		}
		block_cnt++;
		if (block_cnt == need_block_cnt) {
			break;
		}
	}
	if (start_index == -1) {
		printf("No available memory to used\n");
		return NULL;
	}
	alloc_table = alloc_talbe_pos(pool);
	for (i = 0; i < pool->block_cnt; i++) {
		p_alloc_table = (Memory_Alloc_Table *)((char *)alloc_table + i * sizeof(Memory_Alloc_Table));
		if (p_alloc_table->used == 0) {
			break;
		}
		p_alloc_table = NULL;
	}
	if (p_alloc_table == NULL) {
		return NULL;
	}
	p_map_table = (Memory_Map_Table *)((char *)map_table + sizeof(Memory_Map_Table) * start_index);
	p_alloc_table->p_start = p_map_table->p_block;
	p_alloc_table->block_start_index = p_map_table->index;
	p_alloc_table->block_cnt = block_cnt;
	p_alloc_table->used = 1;
	//printf("block cnt is %d\n", block_cnt);
	for (i = start_index; i < start_index + block_cnt; i++) {
		p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
		//printf("map index: %d is used\n", i);
		p_map_table->used = 1;
	}
	printf("Alloc size: %d, Block: (start: %d, end: %d, cnt: %d)\n", size, start_index, start_index + block_cnt - 1, block_cnt);
    pool->alloc_cnt++;
	pool->used_size += size;
	return p_alloc_table->p_start;

}
void memory_free(Memory_Pool *pool, void *memory)
{
	Memory_Map_Table *map_table = NULL;
	Memory_Map_Table *p_map_table = NULL;
	Memory_Alloc_Table *alloc_table = NULL;
	Memory_Alloc_Table *p_alloc_table = NULL;
	int i = 0;
	int block_start_index = 0;
	int block_cnt = 0;
	if (memory == NULL) {
		printf("memory_free(): memory is NULL\n");
		return;
	}
	if (pool == NULL) {
		printf("Pool is NULL\n");
		return;
	}
    alloc_table = alloc_talbe_pos(pool);
	for (i = 0; i < pool->alloc_cnt; i++) {
		p_alloc_table = (Memory_Alloc_Table *)((char *)(alloc_table) + i * sizeof(Memory_Alloc_Table));
		if (p_alloc_table->p_start == memory) {
			block_start_index = p_alloc_table->block_start_index;
			block_cnt = p_alloc_table->block_cnt;
		}
	}
	if (block_cnt == 0) {
		return;
	}
    map_table = map_table_pos(pool);
	printf("Block free: (start: %d, end: %d, cnt: %d)\n", block_start_index, block_start_index + block_cnt -1, block_cnt);
    for (i = block_start_index; i < block_start_index + block_cnt; i++) {
		p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
		p_map_table->used = 0;
	}
	p_alloc_table->used = 0;
	pool->used_size -= block_cnt * BLOCK_SIZE;
}
void memory_pool_destroy(Memory_Pool *pool)
{
	if (pool == NULL) {
		printf("memory_pool_destroy: pool is NULL\n");
		return;
	}
	free(pool);
	pool = NULL;
}