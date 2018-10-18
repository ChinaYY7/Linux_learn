//gcc main.c Pthread_count.c Tree.c -o ../../bin/Pthread_count.out -lpthread
//"c": "cd $dir && gcc -g main.c Pthread_count.c -o ../../bin/Pthread_count.out -lpthread && cd $dir/../../bin && ./Pthread_count.out",
//scp -P 8022 scp u0_a928@192.168.1.5:/data/data/com.termux/files/home/Linux_code/code/Pthread_count.c /mnt/f/Linux_code/Termux
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"
#include "Tree.h"

struct queue Job_queue;
struct job Thread_job [Job_num];
struct msg Thread_msg[Thread_num + 1];
Vocabulary_info Vocabulary_info_result[Thread_num + 1];

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

    Vocabulary_info tmp;
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
            //Str_num += Count_str_num(Now_job->File_Path);
            Tree_Count_str_num(Now_job->File_Path, &tmp);
            Merge_Vocabulary_Result(&Vocabulary_info_result[i],&tmp);
            Now_job->Free = True;
        }
    }
    //Thread_msg[i].Str_num = Str_num;
    Thread_msg[i].Vocabulary = &Vocabulary_info_result[i];
    Thread_msg[i].END = False;
    enqueue_msg(&Thread_msg[i]);
    printf("pid %lu->Count thread %d tid %lu (0x%1lx):  Sum = %d,  Kind = %d,  Repetition = %d,  Average = %.2f,  Max = %d(%s),  Min = %d(%s)\n",
    (unsigned long)pid,i,(unsigned long)tid,(unsigned long)tid,Vocabulary_info_result[i].Sum,Vocabulary_info_result[i].Kind_vocabulary_num,
    Vocabulary_info_result[i].Repetition_vocabulary_num,Vocabulary_info_result[i].Average_repetition_num,Vocabulary_info_result[i].Max_repetition_num
    ,Vocabulary_info_result[i].Max_repetition_vocabulary,Vocabulary_info_result[i].Min_repetition_num,Vocabulary_info_result[i].Min_repetition_vocabulary);

    //printf("pid %lu -> Count thread %d tid %lu (0x%1lx): Str_num = %d\n",(unsigned long)pid,i,(unsigned long)tid,(unsigned long)tid,Str_num);
    pthread_barrier_wait(&b);
    return ((void *) 1);
}
void *Thr_Sum(void *arg)
{
    pid_t pid;
    pthread_t tid;
    struct msg *Thread_receive_msg;
    int Str_num = 0;
    Bool First_sta = True;

    pid = getpid();
    tid = pthread_self();
    while(1)
    {
        Thread_receive_msg = process_msg();
        //Thread_receive_msg->Vocabulary->Sum;
        if(True == Thread_receive_msg->END)
            break;
        Merge_Vocabulary_Result(&Vocabulary_info_result[Thread_num],Thread_receive_msg->Vocabulary);
    }
    //printf("\npid %lu -> Sum thread tid %lu (0x%1lx): Str_num = %d\n",(unsigned long)pid,(unsigned long)tid,(unsigned long)tid,Str_num);
    printf("\npid %lu->Sum thread tid %lu (0x%1lx):  Sum = %d,  Kind = %d,  Repetition = %d,  Average = %.2f,  Max = %d(%s),  Min = %d(%s)\n",
    (unsigned long)pid,(unsigned long)tid,(unsigned long)tid,Vocabulary_info_result[Thread_num].Sum,Vocabulary_info_result[Thread_num].Kind_vocabulary_num,
    Vocabulary_info_result[Thread_num].Repetition_vocabulary_num,Vocabulary_info_result[Thread_num].Average_repetition_num,Vocabulary_info_result[Thread_num].Max_repetition_num
    ,Vocabulary_info_result[Thread_num].Max_repetition_vocabulary,Vocabulary_info_result[Thread_num].Min_repetition_num,Vocabulary_info_result[Thread_num].Min_repetition_vocabulary);
    return ((void *) 1);
}


int main(void)
{
    int err,i;
    pthread_t tid[Thread_num + 1];
    void *tret;
    int Str_num,Free_job_suffix = Job_num;
    int arg[Thread_num][2];
    char File_path[] = Path;
    struct timeval start,finish;
    double Complate_time;

    pthread_barrier_init(&b, NULL, Thread_num + 1);
    queue_init(&Job_queue);
    Thread_job_init(Thread_job);
    Vocabulary_Info_Result_Init(Vocabulary_info_result, Thread_num + 1);

    Generate_test_tmp(File_num,Str_count);
    gettimeofday(&start,NULL);//获取程序开始时间
    for(i = 0; i < Thread_num; i++)
    {
        arg[i][0]=i;
        if(pthread_create(&tid[i], NULL, Thr_Count, (void *)arg[i]) != 0)
            Error_Exit("can't create thread");
    }
    if(pthread_create(&tid[Thread_num], NULL, Thr_Sum, (void *)arg[i]) != 0)
            Error_Exit("can't create thread");

    for(i = 0; i < File_num; i++)
    {
        #if Termux == 1
        File_path[3] = '0' + i;
        #else
        File_path[30] = '0' + i;
        #endif
        do
            Free_job_suffix = Find_Free_job(Thread_job);
        while(Free_job_suffix == Job_num);
        strcpy(Thread_job[Free_job_suffix].File_Path,File_path);
        Thread_job[Free_job_suffix].j_id = i % Thread_num;
        Thread_job[Free_job_suffix].END = False;
        Thread_job[Free_job_suffix].Free = False;
        job_append(&Job_queue,&Thread_job[Free_job_suffix]);
    }
    
    for(i = 0; i < Thread_num; i++)
    {
        do
            Free_job_suffix = Find_Free_job(Thread_job);
        while(Free_job_suffix == Job_num);
        Thread_job[Free_job_suffix].j_id = i;
        Thread_job[Free_job_suffix].END = True;
        Thread_job[Free_job_suffix].Free = False;
        job_append(&Job_queue,&Thread_job[Free_job_suffix]);
    }
    pthread_barrier_wait(&b);

    Thread_msg[Thread_num].END = True;
    enqueue_msg(&Thread_msg[Thread_num]);

    if (pthread_join(tid[Thread_num], NULL) != 0)
        Error_Exit("can't join with thread 1");
    gettimeofday(&finish,NULL);
    Complate_time = (double)((finish.tv_sec-start.tv_sec) * 1000000 + (finish.tv_usec-start.tv_usec)) / 1000000;
    printf("\nCount complated !\ntime: %.3f s\n",Complate_time);
    Remove_tmp_file(File_num);
    exit(0);
}