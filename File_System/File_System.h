#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H
#include "apue.h"
#define DEBUG 0

#define TYPE_REG 1
#define TYPE_DIR 2

#define FILE_SYSTEM_SIZE 4294967296 //4GB = 4 X 1024 X 1024 X 1024B = 2^32 B
#define BLOCK_SIZE 4096 //4KB
#define FREE_BLOCK_TABLE_SIZE (BLOCK_SIZE / 4) 

#define DISK_INODE_NUM 65536 //2^16
#define DISK_FREE_INODE_TABLE_SIZE 256
#define MEM_FREE_INODE_TABLE_SIZE 256

#define NAME_MAX_LENGTH 56
#define MAX_DIRENT_ITEM 256

#define File_System_ID "YY_File_System\0"

#define INDEX_BLOCK_FREE -1

typedef struct Surper_Block_Struct{
    char ID[20];

    int Free_Block_Num;
    int Free_Block_Table[FREE_BLOCK_TABLE_SIZE];  //1M空间
    short int Free_Block_Suffix;

    int Free_Inode_Num;
    int Free_Inode_Table[DISK_FREE_INODE_TABLE_SIZE];
    short int Free_Inode_Suffix;

    int Start_Block_Offset;
    int Start_Inode_Offset;
    int Extend_Size;

    int Dirent_Root_Inode_Num;
    
    Bool Modified;
}Surper_Block;

typedef struct Disk_INode_Struct{
    char File_Type;
    char File_Modifie_Time[30];
    char File_Creat_Time[30];
    char File_Access_Time[30];
    char Inode_Modifie_Time[30];
    unsigned int File_Size;
    int File_Name_Link;
    int File_Disk_Address[13];
}Disk_Inode;

typedef struct Mem_INode_Struct{
    int Inode_Num;
    Bool Modified_Inode;
    Bool Modified_File;
    char File_Type;
    char File_Modifie_Time[30];
    char File_Creat_Time[30];
    char File_Access_Time[30];
    char Inode_Modifie_Time[30];
    unsigned int File_Size;
    int File_Name_Link;
    int File_Open_Link;
    int File_Disk_Address[13];
    struct Mem_INode_Struct *Next;
    struct Mem_INode_Struct *Next_Free;
    struct Mem_INode_Struct *Prior_Free;
}Mem_Inode, *Mem_Inode_P;

typedef struct Dirent_Item_Struct{
    int Num;
    char Name[NAME_MAX_LENGTH];
    int Inode_Num;
}Dirent_Item, *Dirent_Item_P;


int Block_Num_to_Offset(int Block_Num);
int Inode_Num_to_Offset(int Inode_Num);
int Read_Block_Int_Full(int Block_Num, int *Block_Buf);
int Read_Block_Int_One(int Block_Num, int *One_Int, int Int_num);
int Write_Block_Int(int Block_Num, int *Block_Buf, int Write_Num);
int Write_Block_Int_One(int Block_Num, int *One_Int, int Int_num);


int Balloc(void);
int Bfree(int Block_Num);
int Bfree_Inode(Mem_Inode_P Mem_Inode_Info);

int Index_Block_Init(int Disk_Block_Num);

int Bmap_Direct_Block(int Block_Num, int Block_Offset,Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta);
int Bmap_Indirect1_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta);
int Bmap_Indirect2_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta);
int Bmap_Indirect3_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta);
int File_Bmap(int Address, Mem_Inode_P Mem_Inode_Num, int *Offset, int *Useful_Size, Bool Sta);

int Bmap_Block(int Address, Mem_Inode_P Mem_Inode_Num, int *Offset, int *Useful_Size, Bool Sta);


int Read_Inode(int Inode_Num, Mem_Inode_P Mem_Inode_Info);
int Write_Inode(Mem_Inode_P Mem_Inode_Info);
int Refresh_Inode(Mem_Inode_P Mem_Inode_Info);
Mem_Inode_P Iget(int Inode_Num);
int Search_Free_Disk_Inode(void);
Mem_Inode_P Ialloc(void);
void Ifree(int Inode_Num);
void Iput(Mem_Inode_P Mem_Inode_Info);


int Creat_Mem_Inode_Table(void);
int Refresh_Surper_Block(void);
int Surper_Block_Init(void);
int Free_Block_Table_Init(void);
int Free_Inode_Table_Init(void);
int Creat_File_System(void);
int Read_File_System(void);
int Print_File_System_Info(void);
int Init_File_System_Part(void);
void Get_Time(char *Time_Buf);


int Write_Dirent_Item(Mem_Inode_P Mem_Inode_Num, Dirent_Item Dirent_Item_S);
int Creat_Root_Dirent(void);
int Get_Path_Name(char *Full_Path, char *Path_Name, int Order);
int Get_Path_Len(char *Path_Name);
int Get_Path_Inode(char *Path_Name, Mem_Inode_P *Opendir_Inode_Info);
int Delete_Dirent_Item(Mem_Inode_P Dir_Inode_Info, Dirent_Item Dirent_Item_S);
void Remove_File_Name(char *Path_Name, char *File_Name);
int Inode_To_Name(int Inode_Num, char *Name);


int Open_File_System(char *Path);
int Close_File_System(void);
void Get_Current_Name(char *Current_Dirent_Name);

int Opendir_S(char *Path_Name, Mem_Inode_P *mem_Inode_Info);
int Readir_S(Mem_Inode_P Mem_Inode_Info, Dirent_Item_P Dirent_Item_P, int Dirent_Item_Num);
int Creat_S(char *Path_Name);
int Mkdir_S(char *Path_Name);
int Open_S(char *Path_Name, Mem_Inode_P *mem_Inode_Info);
void Close_s(Mem_Inode_P Mem_Inode_Info);
int Write_S(Mem_Inode_P Mem_Inode_Info, char *Str_Buf, int Write_Num);
int Read_S(Mem_Inode_P Mem_Inode_Info, char *Str_Buf, int Read_Num);
int Unlink_S(char *Path_Name);
int File_Stat(char *Path_Name);
int ls(char cmd, char *Path_Name);
int Cd_S(char *Path_Name);


#endif
