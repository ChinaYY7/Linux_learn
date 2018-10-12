//gcc Pthread_count.c -o ../bin/Pthread_count.out -lpthread
//"c": "cd $dir && gcc -g $fileName -o ../bin/$fileNameWithoutExt.out -lpthread && cd $dir/../bin && ./$fileNameWithoutExt.out",
//scp -P 8022 scp u0_a928@192.168.1.5:/data/data/com.termux/files/home/Linux_code/code/Pthread_count.c /mnt/f/Linux_code/Termux
#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 

#define Termux 0
#if Termux == 1
#define Path "tmp0.tmp"
#else
#define Path "/mnt/f/Linux_code/tmp/Tmp_File0.tmp"
#endif
struct job{
    struct job *j_next;
    struct job *j_prev;
    pthread_t j_id;
    char END;
    char File_Path[100];
    int Offset_start;
    int Char_num;
};

struct queue{
    struct job *q_head;
    struct job *q_tail;
    pthread_rwlock_t q_lock;
};

int queue_init(struct queue *qp)
{
    int err;
    qp->q_head = NULL;
    qp->q_tail = NULL;
    err = pthread_rwlock_init(&qp->q_lock,NULL);
    if(err != 0)
    {
        printf("rwlock_init_fail\n");
        return (err);
    }
    return 0;
}

void job_insert(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    jp->j_next = qp->q_head;
    jp->j_prev = NULL;
    if(qp->q_head != NULL)
        qp->q_head->j_prev = jp;
    else
        qp->q_tail = jp;
    qp->q_head = jp;
    pthread_rwlock_unlock(&qp->q_lock);
}

void job_append(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    jp->j_next = NULL;
    jp->j_prev = qp->q_tail;
    if(qp->q_tail != NULL)
        qp->q_tail->j_next = jp;
    else
        qp->q_head = jp;
    qp->q_tail = jp;
    pthread_rwlock_unlock(&qp->q_lock);
}

void job_remove(struct queue *qp, struct job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    if(jp == qp->q_head)
    {
        qp->q_head = jp->j_next;
        if(qp->q_tail == jp)
            qp->q_tail = NULL;
        else
            jp->j_next->j_prev = jp->j_prev;
    }
    else if(jp == qp->q_tail)
    {
        qp->q_tail = jp->j_prev;
        jp->j_prev->j_next = jp->j_next;
    }
    else
    {
        jp->j_prev->j_next = jp->j_next;
        jp->j_next->j_prev = jp->j_prev;
    }
    pthread_rwlock_unlock(&qp->q_lock);
}

struct job *job_find(struct queue *qp, pthread_t id)
{
    struct job *jp;
    if(pthread_rwlock_rdlock(&qp->q_lock)!=0)
        return (NULL);
    for(jp = qp->q_head; jp != NULL; jp = jp->j_next)
        if(pthread_equal(jp->j_id, id))
            break;
    pthread_rwlock_unlock(&qp->q_lock);
    return (jp);
}

struct msg{
    struct msg *m_next;
};

struct msg *workq;

pthread_cond_t qready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

void enqueue_msg(struct msg *mp)
{
    pthread_mutex_lock(&qlock);
    mp->m_next = workq;
    workq = mp;
    pthread_mutex_unlock(&qlock);
    pthread_cond_signal(&qready); 
}

void process_msg(void)
{
    struct msg *mp;
    for(;;)
    {
        pthread_mutex_lock(&qlock);
        while(workq == NULL)
            pthread_cond_wait(&qready,&qlock);
        mp = workq;
        workq = mp->m_next;
        pthread_mutex_unlock(&qlock);
    }
}

//出错处理
void Error_Exit(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

void printids(const char *s)
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%1lx)\n",s,(unsigned long)pid,(unsigned long)tid,(unsigned long)tid);
}

void Generate_test_tmp(int file_num, int str_num)
{
    int tmp,i = 0,j;
    int str_count,write_num = 0,str_num_var = str_num;
    FILE *Tmp_File_fp[file_num];
    char File_path[] = Path;
    char str[11];
    srand(time(NULL));
    if(file_num >= 10)
        file_num = 9;
    while(file_num--)
    {
        if((Tmp_File_fp[i] = fopen(File_path,"w")) == NULL)
            Error_Exit("Creat target file error\n");
        str_num_var = str_num;
        while(str_num_var--)
        {
            str_count = rand() % 10;
            if(str_count == 0 || str_count == 1)
                str_count = 2;
            for(j = 0; j < str_count; j++)
                str[j] = 'a'+rand() % 26;
            write_num++;
        
            if(write_num % 20 == 0)
            {
                str[j] = '\n';
                str[j + 1] = ' ';
                fwrite(str,sizeof(char),j+2,Tmp_File_fp[i]);
            }
            else if(write_num % 33 == 0)
            {
                str[j] = '\t';
                str[j + 1] = ' ';
                fwrite(str,sizeof(char),j+2,Tmp_File_fp[i]);
            }
            else
            {
                str[j] = ' ';
                fwrite(str,sizeof(char),j+1,Tmp_File_fp[i]);
            }
        }
        fclose(Tmp_File_fp[i]);
        i++;
#if Termux == 1
        File_path[3] = '0' + i;
#else
        File_path[30] = '0' + i;
#endif 
    }
}
int Count_str_num(char *File_path)
{
    char Str_tmp,Str_num_sta = 1;
    int Read_count,Str_num = 0;
    FILE *Tmp_File_fp;
    if((Tmp_File_fp = fopen(File_path,"r")) == NULL)
        Error_Exit("Can not open file\n");
    while(fread(&Str_tmp,sizeof(char),1,Tmp_File_fp))
    {
        //printf("%c",Str_tmp);
        if((Str_tmp < 'a' || Str_tmp > 'z') && Str_num_sta)
        {
            Str_num_sta = 0;
            Str_num++;
        }
        else
            Str_num_sta = 1; 
    }
    return Str_num;
}

#define Thread_num 4
struct queue Job_queue;
struct job Thread_job [12];
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
