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
    Vocabulary_info Vocabulary_info_result;
    Generate_test_tmp(File_num,Str_count);
    Tree_Count_str_num(Path, &Vocabulary_info_result);
    printf("Sum = %d,  Kind = %d,  Repetition = %d,  Average = %.2f,  Max = %d(%s),  Min = %d(%s)\n",Vocabulary_info_result.Sum,Vocabulary_info_result.Kind_vocabulary_num,
    Vocabulary_info_result.Repetition_vocabulary_num,Vocabulary_info_result.Average_repetition_num,Vocabulary_info_result.Max_repetition_num
    ,Vocabulary_info_result.Max_repetition_vocabulary,Vocabulary_info_result.Min_repetition_num,Vocabulary_info_result.Min_repetition_vocabulary);
    return 0;
}
