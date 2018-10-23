#include "apue.h"
#include <pthread.h>
#include <sys/time.h> 
#include "Pthread_count.h"

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

struct msg *process_msg(void)
{
    struct msg *mp;
    
    pthread_mutex_lock(&qlock);
    while(workq == NULL)
        pthread_cond_wait(&qready,&qlock);
    mp = workq;
    workq = mp->m_next;
    pthread_mutex_unlock(&qlock);

    return mp;
    
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
    printf("%spid %lu tid %lu (0x%1lx)\n",s,(unsigned long)pid,(unsigned long)tid,(unsigned long)tid);
}

void Generate_test_tmp(int file_num, int str_num)
{
    int tmp,i = 0,j;
    int str_count,write_num = 0,str_num_var = str_num;
    FILE *Tmp_File_fp[file_num];
    char File_path[] = Path;
    char str[11];
    srand(time(NULL));
    while(file_num--)
    {
        if((Tmp_File_fp[i] = fopen(File_path,"w")) == NULL)
            Error_Exit("Creat target file error\n");
        str_num_var = str_num;
        while(str_num_var--)
        {
            str_count = rand() % STR_MAX;
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

void Remove_tmp_file(int file_num)
{
    char File_path[] = Path;
    while(file_num--)
    {
        if(remove(File_path))
            Error_Exit("Can't delete file\n");
#if Termux == 1
        File_path[3] = '0' + file_num;
#else
        File_path[30] = '0' + file_num;
#endif 
    }
}

void Thread_job_init(struct job *Thread_job)
{
    int i;
    for (i = 0; i < Job_num; i++)
    {
        Thread_job[i].Free = True;
        Thread_job[i].Path_men_sta = False;
    }
        
}

int Find_Free_job(struct job *Thread_job)
{
    int i;
    for (i = 0; i < Job_num; i++)
    {
        if(True == Thread_job[i].Free)
            break;
    }
    return i;
}

void Vocabulary_Insert_Tree(char *File_path, PtrT T)
{
    char Str_tmp,Insert_sta = 0;
    int Str_num = 0;
    FILE *Tmp_File_fp;
    char String[STR_MAX + 1];
    int i = 0;

    if((Tmp_File_fp = fopen(File_path,"r")) == NULL)
        Error_Exit("Can not open file\n");
    while(fread(&Str_tmp,sizeof(char),1,Tmp_File_fp))
    {
        if((Str_tmp >= 'a' && Str_tmp <= 'z') || (Str_tmp >= 'A' && Str_tmp <= 'Z') || Str_tmp == 39)
        {
            String[i] = Str_tmp;
            i++;
            String[i] = '\0';
            Insert_sta = 1;
        }
        else
        {
            if(Insert_sta == 1)
            {
                TInsert(T, String);
                Insert_sta = 0;
                i = 0;
            }
        }
    }
    if(Insert_sta == 1)  //最后的词汇后面可能没有空格等
        TInsert(T, String);
    fclose(Tmp_File_fp);
}

void Print_Vocabulary_Result(Vocabulary_info Vocabulary)
{
    printf("Sum = %ld,  Kind = %d,  Repetition = %d,  Average = %.2f,  Max = %d(%s),  Min = %d(%s)\n",Vocabulary.Sum,Vocabulary.Kind_vocabulary_num,
    Vocabulary.Repetition_vocabulary_num,Vocabulary.Average_repetition_num,Vocabulary.Max_repetition_num
    ,Vocabulary.Max_repetition_vocabulary,Vocabulary.Min_repetition_num,Vocabulary.Min_repetition_vocabulary);
}

void Assign_END_Job(struct queue *Job_queue, struct job *Thread_job)
{
    int i;
    int Free_job_suffix = Job_num;
    for(i = 0; i < Thread_num; i++)
    {
        do
            Free_job_suffix = Find_Free_job(Thread_job);
        while(Free_job_suffix == Job_num);
        Thread_job[Free_job_suffix].j_id = i;
        Thread_job[Free_job_suffix].END = True;
        Thread_job[Free_job_suffix].Free = False;
        job_append(Job_queue,&Thread_job[Free_job_suffix]);
    }
}