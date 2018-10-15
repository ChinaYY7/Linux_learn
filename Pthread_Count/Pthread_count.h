#ifndef _PTHREAD_COUNT_H
#define _PTHREAD_COUNT_H

#define Termux 0
#define Thread_num 5
#define File_num 50
#define Str_count 10000
#define Job_num Thread_num + 2
#define queuesize (Thread_num + 2)


#if Termux == 1
#define Path "tmp0.tmp"
#else
#define Path "/mnt/f/Linux_code/tmp/Tmp_File0.tmp"
#endif

typedef enum BoolType{
    False = 0,
    True = 1
}Bool;

struct job{
    struct job *j_next;
    struct job *j_prev;
    pthread_t j_id;
    Bool END;
    Bool Free;
    char File_Path[100];
    int Offset_start;
    int Char_num;
};

struct queue{
    struct job *q_head;
    struct job *q_tail;
    pthread_rwlock_t q_lock;
};

struct Free_queue
{
	int head, tail; 
	struct job **Thread_job;          
};
struct msg{
    struct msg *m_next;
    int Str_num;
    Bool END;
};

int queue_init(struct queue *qp);
void job_insert(struct queue *qp, struct job *jp);
void job_append(struct queue *qp, struct job *jp);
void job_remove(struct queue *qp, struct job *jp);
struct job *job_find(struct queue *qp, pthread_t id);
void enqueue_msg(struct msg *mp);
struct msg *process_msg(void);
void Error_Exit(const char *str);
void printids(const char *s);
void Generate_test_tmp(int file_num, int str_num);
void Remove_tmp_file(int file_num);
int Find_Free_job(struct job *Thread_job);
void Thread_job_init(struct job *Thread_job);

int Count_str_num(char *File_path);
void Init_Free_Queue(struct Free_queue *q);
void DeleteQueue(struct Free_queue *q);
void EnQueue(struct Free_queue *q, struct job *Free_job);


struct job *DeQueue(struct Free_queue *q);


#endif