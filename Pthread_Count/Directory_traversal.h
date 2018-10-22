#ifndef _DIRECTORY_TRAVERSAL_C
#define _DIRECTORY_TRAVERSAL_C

#include "apue.h"

#define FTW_F 1     //表示不是目录的文件
#define FTW_D 2     //目录文件
#define FTW_DNR 3   //目录不能打开，没权限
#define FTW_NS 4    //不能读取文件信息

void Allocate_Pathname_Mem(char *Pathname);
int Descending_Traversal_Directory(void);
int Process_File_Type(const char *Pathname, const struct stat *statptr, int type);
void Print_Find_Result(char *Pathname);

void Find_Reg_File(char *Pathname);

void Assign_Job(const char *Pathname);


#endif