#include "Tree.h"
//为节点分配内存
//返回值：为节点分配的内存的地址
CPtrTNode TAllocateNode(char Alphabet)
{
    int i;

    CPtrTNode NewNode = (CPtrTNode)malloc(sizeof(CTNode));
    NewNode->Alphabet = Alphabet;
    NewNode->num = 0;
    for(i = 0; i < Alphabet_num; i++)
        NewNode->Child[i] = NULL;
    
    return NewNode;
}

//创建树
//入口参数：T树的根目录
void TCreateTree(CPtrT *T)
{
    *T = (CPtrT)malloc(sizeof(struct Tree));

    (*T)->Root = TAllocateNode('0');
}

//初始化队列
void InitQueue(CQueue *q)
{
	q->CQueue_TNode = (CPtrTNode*)malloc(sizeof(CPtrTNode) * queuesize);
	q->tail = 0;
	q->head = 0;
}

//删除队列
void DeleteQueue(CQueue *q)
{
    //int i;
    //for( i = 0; i < queuesize; i++)
        free(q -> CQueue_TNode);
}

//入队
void EnQueue(CQueue *q, CPtrTNode CTNode)
{
	int tail = (q->tail + 1) % queuesize; 
	if (tail == q->head)                   
	{
		printf("\nthe CQueue has been filled full!\n");
        exit(0);
	}
	else
	{
        q->CQueue_TNode[q->tail] = CTNode;
		q->tail = tail;
	}
}

//出队
CPtrTNode DeQueue(CQueue *q)
{
	CPtrTNode tmp;
	if (q->tail == q->head)     
	{
		printf("the CQueue is NULL\n");
        exit(0);
	}
	else
	{
		tmp = q->CQueue_TNode[q->head];
		q->head = (q->head + 1) % queuesize;
	}
	return tmp;
}

//插入节点
//入口参数：T根节点，Val插入的关键字
void TInsert(CPtrT T, char *String)
{
    int i = 0;
    CPtrTNode NewNode;
    CQueue TNode_Queue;
    CPtrTNode tmp;

    InitQueue(&TNode_Queue);

    EnQueue(&TNode_Queue,T->Root);
    
    while(1)
    {
        tmp = DeQueue(&TNode_Queue);
        if(tmp->Child[String[i] - 'a'] == NULL)
        {
            NewNode = TAllocateNode(String[i]);
            tmp->Child[String[i] - 'a'] = NewNode;
            EnQueue(&TNode_Queue,NewNode);
        }
        else
            EnQueue(&TNode_Queue,tmp->Child[String[i] - 'a']);
        i++;
        if('\0' == String[i])
        {
            tmp->Child[String[i - 1] - 'a']->num++;
            break;
        }
    }
    DeleteQueue(&TNode_Queue);
}

//层序遍历
int Ttraversal_level(CPtrTNode Root)
{
    CQueue TNode_Queue;
    CPtrTNode tmp;
    int i;
    int String_sum = 0;

    if(NULL == Root)
        return 0;
    InitQueue(&TNode_Queue);
    EnQueue(&TNode_Queue,Root);
    
    
    while(TNode_Queue.head != TNode_Queue.tail)
    {
        tmp = DeQueue(&TNode_Queue);
        printf("%c ",tmp->Alphabet);
        String_sum += tmp->num;
        for(i = 0; i < Alphabet_num; i++)
        {
            if(tmp->Child[i] != NULL)
                EnQueue(&TNode_Queue,tmp->Child[i]);
        }
        //printf("\n");
    }
    printf("\n\n");
    DeleteQueue(&TNode_Queue);
    return String_sum;
}