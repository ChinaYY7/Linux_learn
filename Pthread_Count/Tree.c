#include "Tree.h"
//为节点分配内存
//返回值：为节点分配的内存的地址
PtrTNode TAllocateNode(char Alphabet)
{
    int i;

    PtrTNode NewNode = (PtrTNode)malloc(sizeof(TNode));
    NewNode->Alphabet = Alphabet;
    NewNode->num = 0;
    for(i = 0; i < Alphabet_num; i++)
        NewNode->Child[i] = NULL;
    
    return NewNode;
}

//创建树
//入口参数：T树的根目录
void TCreateTree(PtrT *T)
{
    *T = (PtrT)malloc(sizeof(struct Tree));

    (*T)->Root = TAllocateNode('0');
}

void Destroy_Tree(PtrT *T)
{
    Tree_Queue TNode_Queue;
    PtrTNode tmp;
    int i;

    if(NULL == (*T)->Root)
        exit(0);
    InitQueue(&TNode_Queue);
    EnQueue(&TNode_Queue,(*T)->Root);

    while(TNode_Queue.head != TNode_Queue.tail)
    {
        tmp = DeQueue(&TNode_Queue);
        for(i = 0; i < Alphabet_num; i++)
        {
            if(tmp->Child[i] != NULL)
                EnQueue(&TNode_Queue,tmp->Child[i]);
        }
        free(tmp);
    }
    DeleteQueue(&TNode_Queue);
}

//初始化队列
void InitQueue(Tree_Queue *q)
{
	q->Tree_Queue_TNode = (PtrTNode*)malloc(sizeof(PtrTNode) * queuesize);
	q->tail = 0;
	q->head = 0;
}

//删除队列
void DeleteQueue(Tree_Queue *q)
{
    free(q -> Tree_Queue_TNode);
}

//入队
void EnQueue(Tree_Queue *q, PtrTNode TNode)
{
	int tail = (q->tail + 1) % queuesize; 
	if (tail == q->head)                   
	{
		printf("\nthe Tree_Queue has been filled full!\n");
        exit(0);
	}
	else
	{
        q->Tree_Queue_TNode[q->tail] = TNode;
		q->tail = tail;
	}
}

//出队
PtrTNode DeQueue(Tree_Queue *q)
{
	PtrTNode tmp;
	if (q->tail == q->head)     
	{
		printf("the Tree_Queue is NULL\n");
        exit(0);
	}
	else
	{
		tmp = q->Tree_Queue_TNode[q->head];
		q->head = (q->head + 1) % queuesize;
	}
	return tmp;
}

//插入节点
//入口参数：T根节点，Val插入的关键字
void TInsert(PtrT T, char *String)
{
    int i = 0;
    PtrTNode NewNode;
    Tree_Queue TNode_Queue;
    PtrTNode tmp;

    InitQueue(&TNode_Queue);

    EnQueue(&TNode_Queue,T->Root);
    
    while(1)
    {
        tmp = DeQueue(&TNode_Queue);
        if(tmp->Child[String[i] - 'a'] == NULL)
        {
            NewNode = TAllocateNode(String[i]);
            tmp->Child[String[i] - 'a'] = NewNode;
            NewNode->Parent = tmp;
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
void Trace_back_vocabulary(PtrTNode Root, PtrTNode Now_Node, char *Vocabulary)
{
    int top = 0,i = 0;
    PtrTNode tmp;
    char str[STR_MAX + 1];

    tmp = Now_Node;
    while(1)
    {
        str[top++] = tmp->Alphabet;
        tmp = tmp->Parent;
        if(Root == tmp)
            break;
    }
    while(1)
    {   
        Vocabulary[i++] = str[--top];
        if(top == 0)
        {
            Vocabulary[i] = '\0';
            break;
        }
    }
}
//层序遍历
void Ttraversal_level(PtrTNode Root, Vocabulary_info *Vocabulary)
{
    Tree_Queue TNode_Queue;
    PtrTNode tmp;
    int i;
    int String_sum = 0;
    Bool First_sta = True;

    if(NULL == Root)
        exit(0);
    InitQueue(&TNode_Queue);
    EnQueue(&TNode_Queue,Root);

    Vocabulary->Kind_vocabulary_num = 0;
    Vocabulary->Repetition_vocabulary_num = 0;

    while(TNode_Queue.head != TNode_Queue.tail)
    {
        tmp = DeQueue(&TNode_Queue);
        
        if(tmp != Root)
        {
            if(tmp->num > 0)
            {
                if(True == First_sta)
                {
                    First_sta = False;
                    Vocabulary->Max_repetition_num = tmp->num;
                    Trace_back_vocabulary(Root,tmp,Vocabulary->Max_repetition_vocabulary);
                    Vocabulary->Min_repetition_num = tmp->num;
                    Trace_back_vocabulary(Root,tmp,Vocabulary->Min_repetition_vocabulary); 
                }
                else
                {
                    if(tmp->num > Vocabulary->Max_repetition_num)
                    {
                        Vocabulary->Max_repetition_num = tmp->num;
                        Trace_back_vocabulary(Root,tmp,Vocabulary->Max_repetition_vocabulary);
                    }
                    if(tmp->num < Vocabulary->Min_repetition_num)
                    {
                        Vocabulary->Min_repetition_num = tmp->num;
                        Trace_back_vocabulary(Root,tmp,Vocabulary->Min_repetition_vocabulary); 
                    }
                }
                Vocabulary->Kind_vocabulary_num++;
                if(tmp->num > 1)
                    Vocabulary->Repetition_vocabulary_num++;
                String_sum += tmp->num;
            }
        }
        //printf("%c ",tmp->Alphabet);
        for(i = 0; i < Alphabet_num; i++)
        {
            if(tmp->Child[i] != NULL)
                EnQueue(&TNode_Queue,tmp->Child[i]);
        }
        //printf("\n");
    }
    //printf("\n\n");
    Vocabulary->Sum = String_sum;
    Vocabulary->Average_repetition_num = (float)String_sum / Vocabulary->Kind_vocabulary_num; 
    DeleteQueue(&TNode_Queue);
}

void Merge_Vocabulary_Result(Vocabulary_info *Vocabulary1, Vocabulary_info *Vocabulary2)
{
    Bool First_sta = True;
    Vocabulary1->Sum += Vocabulary2->Sum;
    Vocabulary1->Kind_vocabulary_num += Vocabulary2->Kind_vocabulary_num;
    Vocabulary1->Repetition_vocabulary_num += Vocabulary2->Repetition_vocabulary_num;

    if(Vocabulary1->Max_repetition_num < Vocabulary2->Max_repetition_num)
    {
        Vocabulary1->Max_repetition_num = Vocabulary2->Max_repetition_num;
        strcpy(Vocabulary1->Max_repetition_vocabulary,Vocabulary2->Max_repetition_vocabulary);
    }
    if(Vocabulary1->Min_repetition_num > Vocabulary2->Min_repetition_num)
    {
        Vocabulary1->Min_repetition_num = Vocabulary2->Min_repetition_num;
        strcpy(Vocabulary1->Min_repetition_vocabulary,Vocabulary2->Min_repetition_vocabulary);
    }
    Vocabulary1->Average_repetition_num = (float)Vocabulary1->Sum / Vocabulary1->Kind_vocabulary_num;
}

void Vocabulary_Info_Result_Init(Vocabulary_info *Vocabulary, int num)
{
    int i;
    for(i = 0; i < num; i++)
    {
        Vocabulary[i].Sum = 0;
        Vocabulary[i].Kind_vocabulary_num = 0;
        Vocabulary[i].Repetition_vocabulary_num = 0;
        Vocabulary[i].Max_repetition_num = 1;
        Vocabulary[i].Min_repetition_num = 10000000;
    }
}