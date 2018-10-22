#ifndef _TREE_H
#define _TREE_H
#include "apue.h"

#define STR_MAX 10
#define queuesize 1000000
#define Alphabet_num 26 

//Tree节点结构体
typedef struct TreeNode{
    int num;
    char Alphabet;
    struct TreeNode *Child[Alphabet_num];
    struct TreeNode *Parent;
}TNode, *PtrTNode;

//Tree根节点结构体
typedef struct Tree{
    PtrTNode Root;
}*PtrT;

//队列结构体

typedef struct Tree_queue
{
	int head, tail; 
	PtrTNode *Tree_Queue_TNode;          
}Tree_Queue;

typedef struct Vocabulary
{
    int Sum;
    int Max_repetition_num;
    int Min_repetition_num;
    float Average_repetition_num;
    int Repetition_vocabulary_num;
    int Kind_vocabulary_num;
    char Max_repetition_vocabulary[STR_MAX + 1];
    char Min_repetition_vocabulary[STR_MAX + 1];
}Vocabulary_info;


void TCreateTree(PtrT *T);
void Destroy_Tree(PtrT *T);
void Ttraversal_level(PtrTNode Root, Vocabulary_info *Vocabulary);
void TInsert(PtrT T, char *String);
PtrTNode TAllocateNode(char Alphabet);
void InitQueue(Tree_Queue *q);
void DeleteQueue(Tree_Queue *q);
void EnQueue(Tree_Queue *q, PtrTNode TNode);
PtrTNode DeQueue(Tree_Queue *q);
void Merge_Vocabulary_Result(Vocabulary_info *Vocabulary1, Vocabulary_info *Vocabulary2);
void Vocabulary_Info_Result_Init(Vocabulary_info *Vocabulary, int num);


#endif