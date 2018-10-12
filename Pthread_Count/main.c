//gcc main.c Pthread_count.c -o ../../bin/Pthread_count.out -lpthread
//"c": "cd $dir && gcc -g main.c Pthread_count.c -o ../../bin/Pthread_count.out -lpthread && cd $dir/../../bin && ./Pthread_count.out",
//scp -P 8022 scp u0_a928@192.168.1.5:/data/data/com.termux/files/home/Linux_code/code/Pthread_count.c /mnt/f/Linux_code/Termux
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"

struct queue Job_queue;
struct job Thread_job [Job_num];

pthread_barrier_t b;

void *Thr_Count(void *arg)
{
    int i,*arg1;
    pid_t pid;
    pthread_t tid;
    struct job *Now_job;
    int Str_num = 0;

    pid = getpid();
    tid = pthread_self();
    arg1 = arg;
    i  = arg1[0];
    while(1)
    {
        Now_job = job_find(&Job_queue,i);
        if(Now_job != NULL)
        {
            job_remove(&Job_queue,Now_job);
            if(Now_job->END)
                break;
            Str_num += Count_str_num(Now_job->File_Path);
        }
    }
    printf("pid %lu -> New thread %d tid %lu (0x%1lx): Str_num = %d\n",(unsigned long)pid,i,(unsigned long)tid,(unsigned long)tid,Str_num);
    pthread_barrier_wait(&b);
    return ((void *) 1);
}

int main(void)
{
    int err,i;
    pthread_t tid[Thread_num];
    void *tret;
    int Str_num;
    int arg[Thread_num][2];
    char File_path[] = Path;

    pthread_barrier_init(&b, NULL, Thread_num + 1);
    queue_init(&Job_queue);


    //Generate_test_tmp(8,2199);

    for(i = 0; i < Thread_num; i++)
    {
        arg[i][0]=i;
        if(pthread_create(&tid[i], NULL, Thr_Count, (void *)arg[i]) != 0)
            Error_Exit("can't create thread");
    }
    
    for(i = 0; i < 8; i++)
    {
        #if Termux == 1
        File_path[3] = '0' + i;
        #else
        File_path[30] = '0' + i;
        #endif
        strcpy(Thread_job[i].File_Path,File_path);
        Thread_job[i].j_id = i % 4;
        Thread_job[i].END = 0;
        job_append(&Job_queue,&Thread_job[i]);
    }
    for(i = 0; i < Thread_num; i++)
    {
        Thread_job[i + 8].j_id = i;
        Thread_job[i + 8].END = 1;
        job_append(&Job_queue,&Thread_job[i + 8]);
    }
    pthread_barrier_wait(&b);
    exit(0);
}