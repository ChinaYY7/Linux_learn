//gcc -g main.c Pthread_count.c Tree.c Directory_traversal.c -o ../../bin/Pthread_count.out -lpthread
//"c": "cd $dir && gcc -g main.c Pthread_count.c Tree.c Directory_traversal.c -o ../../bin/Pthread_count.out -lpthread && cd $dir/../../bin && ./Pthread_count.out /mnt/f/Linux_code/tmp",
//scp -P 8022 scp u0_a928@192.168.1.5:/data/data/com.termux/files/home/Linux_code/code/Pthread_count.c /mnt/f/Linux_code/Termux
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"
#include "Directory_traversal.h"
#include "Tree.h"

struct queue Job_queue;
struct job Thread_job[Job_num];
struct msg Thread_msg[Thread_num + 1];
Vocabulary_info Vocabulary_info_result[Thread_num + 1];

pthread_barrier_t b;

void *Thr_Count(void *arg)
{
    int i,*arg1;
    struct job *Now_job;
    PtrT T_root;

    TCreateTree(&T_root);
    arg1 = arg;
    i  = arg1[0];

    while(1)
    {
        Now_job = job_find(&Job_queue,i);
        if(Now_job != NULL)
        {
            job_remove(&Job_queue,Now_job);
            if(Now_job->END == True)
            {
                Now_job->Free = True;
                break;
            }
            Vocabulary_Insert_Tree(Now_job->File_Path, T_root);
            Now_job->Free = True;
        }
    }

    Thread_msg[i].T = T_root;
    enqueue_msg(&Thread_msg[i]);

    pthread_barrier_wait(&b);
    Destroy_Tree(&T_root);
    return ((void *) 1);
}
void *Thr_Sum(void *arg)
{
    
    int i = 0;

    struct msg *Thread_receive_msg;
    PtrT T_root;

    TCreateTree(&T_root);

    while(1)
    {
        Thread_receive_msg = process_msg();
        Merge_Vocabulary_Tree(T_root->Root,Thread_receive_msg->T->Root);
        i++;
        if(i == Thread_num)
            break;
    }

    Ttraversal_level(T_root->Root,&Vocabulary_info_result[Thread_num]);
    printids("\nStatistical results of word frequency\n");
    Print_Vocabulary_Result(Vocabulary_info_result[Thread_num]);
    pthread_barrier_wait(&b);

    Destroy_Tree(&T_root);

    return ((void *) 1);
}


int main(int argc, char *argv[])
{
    int i;
    pthread_t tid[Thread_num + 1];
    int Free_job_suffix = Job_num;
    int arg[Thread_num][2];
    struct timeval start,finish;
    double Complate_time;

    if (argc != 2)
    {   
        perror("usage: <starting-pathname>");
        exit(0);
    }

    pthread_barrier_init(&b, NULL, Thread_num + 2);
    queue_init(&Job_queue);
    Thread_job_init(Thread_job);
    Vocabulary_Info_Result_Init(Vocabulary_info_result, Thread_num + 1);

    //Generate_test_tmp(File_num,Str_count);
    gettimeofday(&start,NULL);//获取程序开始时间

    for(i = 0; i < Thread_num; i++)
    {
        arg[i][0]=i;
        if(pthread_create(&tid[i], NULL, Thr_Count, (void *)arg[i]) != 0)
            Error_Exit("can't create thread");
    }
    if(pthread_create(&tid[Thread_num], NULL, Thr_Sum, (void *)arg[i]) != 0)
            Error_Exit("can't create thread");

    Find_Reg_File(argv[1]);// /mnt/f/Linux_code/tmp
    Assign_END_Job(&Job_queue,Thread_job);

    pthread_barrier_wait(&b);

    gettimeofday(&finish,NULL);
    Complate_time = (double)((finish.tv_sec-start.tv_sec) * 1000000 + (finish.tv_usec-start.tv_usec)) / 1000000;
    printf("\nCount Path_name: %s complated !\ntime: %.3f s\n",argv[1], Complate_time);
    //Remove_tmp_file(File_num);
    exit(0);
}