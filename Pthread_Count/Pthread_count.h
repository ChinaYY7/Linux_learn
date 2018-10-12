#ifndef _PTHREAD_COUNT_H
#define _PTHREAD_COUNT_H

#define Termux 0
#define Thread_num 4
#define queuesize (Thread_num + 2)
#define Job_num 12


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

struct Free_queue
{
	int head, tail; 
	struct job **Thread_job;          
};
struct msg{
    struct msg *m_next;
};

int queue_init(struct queue *qp);
void job_insert(struct queue *qp, struct job *jp);
void job_append(struct queue *qp, struct job *jp);
void job_remove(struct queue *qp, struct job *jp);
struct job *job_find(struct queue *qp, pthread_t id);
void enqueue_msg(struct msg *mp);
void process_msg(void);
void Error_Exit(const char *str);
void printids(const char *s);
void Generate_test_tmp(int file_num, int str_num);
int Count_str_num(char *File_path);
void Init_Free_Queue(struct Free_queue *q);
void DeleteQueue(struct Free_queue *q);
void EnQueue(struct Free_queue *q, struct job *Free_job);
struct job *DeQueue(struct Free_queue *q);


#endif