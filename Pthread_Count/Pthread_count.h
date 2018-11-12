#ifndef _PTHREAD_COUNT_H
#define _PTHREAD_COUNT_H
#include "apue.h"
#include "Tree.h"

#define Thread_num 3
#define File_num 10
#define Str_count 20000
#define STR_MAX 20
#define Job_num Thread_num + 2


#if Termux == 1
#define Path "tmp0.tmp"
#else
#define Path "/mnt/f/Linux_code/tmp/Tmp_File0.tmp"
#endif

struct job{
    struct job *j_next;
    struct job *j_prev;
    pthread_t j_id;
    Bool END;
    Bool Free;
    char *File_Path;
    int Path_Len;
    Bool Path_men_sta;
};

struct queue{
    struct job *q_head;
    struct job *q_tail;
    pthread_rwlock_t q_lock;
};


struct msg{
    struct msg *m_next;
    PtrT T;
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

void Vocabulary_Insert_Tree(char *File_path, PtrT T);
void Print_Vocabulary_Result(Vocabulary_info Vocabulary);
void Assign_END_Job(struct queue *Job_queue, struct job *Thread_job);

#endif
