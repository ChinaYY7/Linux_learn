#ifndef _TREE_H
#define _TREE_H

#include "apue.h"

#define queuesize 1000000
#define Alphabet_num 26 

//Tree节点结构体
typedef struct CTreeNode{
    int num;
    char Alphabet;
    struct CTreeNode *Child[Alphabet_num];
}CTNode, *CPtrTNode;

//Tree根节点结构体
typedef struct Tree{
    CPtrTNode Root;
}*CPtrT;

//队列结构体

typedef struct Tree_queue
{
	int head, tail; 
	CPtrTNode *CQueue_TNode;          
}CQueue;


void TCreateTree(CPtrT *T);
int Ttraversal_level(CPtrTNode Root);
void TInsert(CPtrT T, char *String);
CPtrTNode TAllocateNode(char Alphabet);
void InitQueue(CQueue *q);
void DeleteQueue(CQueue *q);
void EnQueue(CQueue *q, CPtrTNode CTNode);
CPtrTNode DeQueue(CQueue *q);


#endif