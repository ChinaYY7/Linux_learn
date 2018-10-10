#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H
#include <stdio.h>
#include <stdlib.h>

#define R 1
#define L 0
#define Success 1
#define Fail 0
#define MinDegree 3
#define queuesize 10000000
#define BT_File_path "/mnt/f/Linux_code/tmp/BT_File.txt"

typedef enum BoolType Bool;
enum BoolType{
    False = 0,
    True = 1
};

//B+_Tree节点结构体
typedef struct TreeNode{
    int Num;
    Bool IsLeaf;
    int* Key;
    struct TreeNode **Child;
    struct TreeNode *next;
    struct TreeNode *prior;
    int* date;
}BTNode, *PtrBTNode;

//B+_Tree根节点结构体
typedef struct Tree{
    PtrBTNode Root;
}*PtrBT;

//队列结构体

typedef struct queue
{
	int head, tail; 
	PtrBTNode *Queue_BTNode;          
}Queue;

//写入节点结构体
typedef struct Node_simple
{
	int key[MinDegree * 2 - 1];
    int Num;
    Bool IsLeaf;         
}Write_Node, Read_Node;

/*
PtrBTNode BTAllocateNode(void);
void ShiftKey(int* Key, Bool Direction, int Begin, int End);
void ShiftChild(PtrBTNode *Child, Bool Direction, int Begin, int End);
void BTChildSplit(PtrBTNode SplitNodeP, int ChildIndex);
int GetIndex(int* Key, int Size, int Val);
void BTInsertNonFull(PtrBTNode CurrentNode, int Val);
void Merge(PtrBT T, PtrBTNode ParentNode, int LeftIndex, int RightIndex, int *Delete_num);
void Write_BTNode(FILE *BT_File_fp, PtrBTNode BTNode);
void Read_BTNode(FILE *BT_File_fp, PtrBTNode BTNode);
void InitQueue(Queue *q);
void DeleteQueue(Queue *q);
void EnQueue(Queue *q, PtrBTNode BTNode);
PtrBTNode DeQueue(Queue *q);
*/

PtrBTNode BTSearch(PtrBTNode Root, int Val, int* Index);
void BTInsert(PtrBT T, int Val);
int BTDelete(PtrBT T, PtrBTNode CurrentNode, int Val, int *Delete_num);
void BTtraversal_level(PtrBTNode Root);
void BTtraversal_leaf(PtrBTNode head);
void Clear_tree(PtrBT T, PtrBTNode head, int *Delete_num);
void BTCreateTree(PtrBT *T, PtrBTNode *head);
void Write_BTree(FILE *BT_File_fp, PtrBTNode Root);
void Read_BTree(FILE *BT_File_fp, PtrBTNode Root, PtrBTNode *head);
void Error_Exit(const char *str);
void Get_Bplus_Tree_Info(PtrBTNode Root, int *Node_num, int *Leaf_node_num, int *All_val_num, int *Leaf_val_num);


#endif