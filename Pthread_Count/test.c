//gcc test.c Pthread_count.c Tree.c -o test.out
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"
#include "Tree.h"
#define Path "/mnt/f/Linux_code/tmp/Tmp_File0.tmp"


int main(void)
{
    int Str_num = 0;
    Generate_test_tmp(File_num,Str_count);
    Str_num += Tree_Count_str_num(Path);
    printf("Str_num = %d\n",Str_num);
    return 0;
}
