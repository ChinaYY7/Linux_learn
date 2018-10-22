//gcc test.c Pthread_count.c Tree.c -o test.out
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"
#include "Tree.h"
#include "Directory_traversal.h"

struct job Thread_job[Job_num];
struct queue Job_queue;

int main(void)
{
    int i;
    Vocabulary_info Vocabulary_info_result;
    Generate_test_tmp(File_num,Str_count);
    PtrT T_root;

    TCreateTree(&T_root);

    queue_init(&Job_queue);
    Thread_job_init(Thread_job);
    Vocabulary_Info_Result_Init(&Vocabulary_info_result, 1);

    Find_Reg_File("/mnt/f/Linux_code/tmp");
    for(i = 0; i < Job_num; i++)
    {
        if(Thread_job[i].Free == False)
        {
            Vocabulary_Insert_Tree(Thread_job[i].File_Path, T_root);
            Thread_job[i].Free = True;
        }
    }
    Ttraversal_level(T_root->Root,&Vocabulary_info_result);
    
    Destroy_Tree(&T_root);
    
    Print_Vocabulary_Result(Vocabulary_info_result);
    return 0;
}
