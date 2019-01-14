#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H
#include "apue.h"

#define MAX_POOL_SIZE 1024 * 1024 * 1024
#define BLOCK_SIZE 64

typedef unsigned int Size_V;
typedef int Num_V;
typedef void Point_V;


typedef struct Struct_Memory_Map_Talbe
{
    Point_V *Block_P;
    Num_V Index;
    Bool Used;
}Memory_Map_Table, *Memory_Map_Table_P;

typedef struct Struct_Memory_Chunk_Table
{
    Point_V *Start_Block_P;
    Num_V Start_Block_Index;
    Num_V Block_Num;
    Bool Used;
}Memory_Chunk_Table, *Memory_Chunk_Table_P;

typedef struct Struct_Memory_Pool
{
    Size_V Total_Size;                  //内存池大小(包括元数据)        
    Size_V Alloc_Memory_Size;           //内存池大小(不包含元数据)
    Size_V Block_Size;                  //内存池块大小
    Num_V Block_Num;                    //内存池块数量

    Memory_Map_Table_P Map_Table;       //映射表指针
    Memory_Chunk_Table_P Chunk_Table;   //chunk分配表指针
    Point_V *Memory_Start;              //分配区指针
    
    Size_V Used_Size;                   //内存池已分配大小
    Num_V Chunk_Num;                    //分配了多少大块（连续小块组成），即分配了多少次
    Num_V Used_Block_Num;               //分配了多少小块
    
    
}Memory_Pool, *Memory_Pool_P;

Memory_Pool_P Memory_Pool_Init(Size_V Need_Size);
void *Memory_Malloc(Memory_Pool_P Pool, Size_V Need_Size);
void Memory_Free(Memory_Pool_P Pool, void **Memory_P);
void Memory_Pool_Destroy(Memory_Pool_P Pool);
void Get_Memory_Info(Memory_Pool_P Pool);

#endif