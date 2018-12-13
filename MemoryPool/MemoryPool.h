#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#define MAX_POOL_SIZE 1024 * 1024 * 1024
#define BLOCK_SIZE 64

typedef struct memory_map_talbe
{
    char *p_block;
    int index;
    int used;

} Memory_Map_Table;

typedef struct memory_alloc_table
{
    char *p_start;
    int used;
    int block_start_index;
    int block_cnt;

}Memory_Alloc_Table;

typedef struct memory_pool
{
    char *memory_start;//内存池起始地址, free整个内存池时使用
    Memory_Alloc_Table *alloc_table;
    Memory_Map_Table *map_table;
    unsigned int total_size;
    unsigned int internal_total_size;
    int increment;
	int used_size;
	int block_size;
	unsigned int block_cnt;
	int alloc_cnt;

} Memory_Pool;

extern Memory_Pool *memory_pool_init(int size, int increment);
extern void *Memory_malloc(Memory_Pool *pool, int size);
extern void memory_free(Memory_Pool *pool, void *memory);
extern void memory_pool_destroy(Memory_Pool *pool);

#endif