#include "apue.h"
#define R 1
#define L 0
#define MinDegree 3
typedef enum BoolType Bool;
enum BoolType{
    False = 0,
    True = 1
};
typedef struct TreeNode *PtrBTNode;
typedef struct TreeNode BTNode;
struct TreeNode{
    int Num;
    Bool IsLeaf;
    int* Key;
    PtrBTNode *Child;
};

struct Tree{
    PtrBTNode Root;
};
typedef struct Tree *PtrBT;

//为节点分配内存
//返回值：为节点分配的内存的地址
PtrBTNode BTAllocateNode(void)
{
    int i;

    PtrBTNode NewNode = (PtrBTNode)malloc(sizeof(BTNode));

    NewNode->Num = 0;
    NewNode->IsLeaf = True;//默认新分配的节点为叶子节点

    NewNode->Key = (int*)malloc(sizeof(int) * (MinDegree * 2 - 1));
    NewNode->Child = (PtrBTNode*)malloc(sizeof(PtrBTNode) * (MinDegree * 2));

    for(i = 0; i < MinDegree * 2; i++)
        NewNode->Child[i] = NULL;
    
    return NewNode;
}

//搜索节点
//入口参数：Root根节点指针，Val查找的关键字，Index被查找关键字所在节点的下标
//返回值：被查找关键字所在节点的指针
PtrBTNode BTSearch(PtrBTNode Root, int Val, int* Index)
{
    int i;

    for(i = 0; i < Root->Num && Val > Root->Key[i]; i++);

    if(i < Root->Num && Root->Key[i] == Val)//当前节点有所查找节点
    {
        *Index = i;
        return Root;
    }
    else if(True == Root->IsLeaf) //整个树中没有所查找节点
        return NULL;
    else
        return BTSearch(Root->Child[i], Val, Index);//递归查找
}

//移动关键字数组内的数据
//入口参数：*Key关键字数组， Direction移动方向，Begin，End需移动数据的范围
void ShiftKey(int* Key, Bool Direction, int Begin, int End)
{
    int i;

    if(R == Direction)
    {
        for(i = End; i >= Begin; i--)
            Key[i + 1] = Key[i];
    }
    else
    {
        for(i = Begin; i <= End; i++)
            Key[i - 1] = Key[i];
    }

}

//移动子节点指针数组内的数据
//入口参数：*Child子节点指针数组， Direction移动方向，Begin，End需移动数据的范围
void ShiftChild(PtrBTNode *Child, Bool Direction, int Begin, int End)
{
    int i;

    if(R == Direction)
    {
        for(i = End; i >= Begin; i--)
            Child[i + 1] = Child[i];
    }
    else
    {
        for(i = Begin; i <= End; i++)
            Child[i - 1] = Child[i];
    }
}

//分裂节点
//入口参数：SplitNodeP需分裂节点的父节点指针，ChildIndex需分裂节点在父节点的子节点指针数组的下标
void BTChildSplit(PtrBTNode SplitNodeP, int ChildIndex)
{

    int i;

    PtrBTNode NewNode = BTAllocateNode(); 
    PtrBTNode FullNode = SplitNodeP->Child[ChildIndex];

    for(i = 0; i < MinDegree - 1; i++)
        NewNode->Key[i] = FullNode->Key[MinDegree + i];

    if(False == FullNode->IsLeaf)
    {
        NewNode->IsLeaf = False;
        for(i = 0; i < MinDegree; i++)
            NewNode->Child[i] = FullNode->Child[MinDegree + i];
    }
    NewNode->Num = FullNode->Num = MinDegree - 1;

    //将被分裂节点中的中间节点插入被分裂节点的父节点中
    ShiftKey(SplitNodeP->Key, R, ChildIndex, SplitNodeP->Num - 1);
    SplitNodeP->Key[ChildIndex] = FullNode->Key[MinDegree - 1];

    ShiftChild(SplitNodeP->Child, R, ChildIndex + 1, SplitNodeP->Num);
    SplitNodeP->Child[ChildIndex + 1] = NewNode;

    (SplitNodeP->Num)++;

}

//取节点中大于等于val的关键字所在数组的下标
//入口参数：*Key关键字数组，Size，该节点关键字数量，Val查找的关键字
int GetIndex(int* Key, int Size, int Val)
{
    int i;

    for(i = 0; i < Size && Val > Key[i]; i++);

    return i;

}

//插入节点到叶节点中，且将在寻找插入位置过程中遇到的关键字已满的节点提前进行分裂，保证找到叶节点中的位置后即可插入，避免回溯
//入口参数：CurrentNode当前查找的节点，Val需要插入的关键字
void BTInsertNonFull(PtrBTNode CurrentNode, int Val)
{

    int Index = GetIndex(CurrentNode->Key, CurrentNode->Num, Val);

    if(True == CurrentNode->IsLeaf) //叶子节点直接插入
    {
        ShiftKey(CurrentNode->Key, R, Index, CurrentNode->Num - 1);
        CurrentNode->Key[Index] = Val;
        (CurrentNode->Num)++;
    }
    else
    {
        if(MinDegree * 2 - 1 == CurrentNode->Child[Index]->Num)
        {
            BTChildSplit(CurrentNode, Index);

            if(CurrentNode->Key[Index] < Val)
                Index++;
        }
        BTInsertNonFull(CurrentNode->Child[Index], Val);
    }
}

//插入节点
//入口参数：T根节点，Val插入的关键字
void BTInsert(PtrBT T, int Val){

    PtrBTNode NewNode;

    //如果根节点中关键字已满
    if(MinDegree * 2 - 1 == T->Root->Num)
    {
        NewNode = BTAllocateNode();
        NewNode->IsLeaf = False;
        NewNode->Child[0] = T->Root;
        T->Root = NewNode;
        BTChildSplit(NewNode, 0);
    }

    BTInsertNonFull(T->Root, Val);
}

//合并节点
//入口参数：T根节点，ParentNode父节点，LeftIndex、RightIndex需合并的节点
void Merge(PtrBT T, PtrBTNode ParentNode, int LeftIndex, int RightIndex)
{
    int i;

    PtrBTNode LeftNode = ParentNode->Child[LeftIndex], RightNode = ParentNode->Child[RightIndex];

    for(i = 0; i < MinDegree - 1; i++)
        LeftNode->Key[MinDegree + i] = RightNode->Key[i];

    if(False == LeftNode->IsLeaf)//不是叶节点还需要合并子节点指针数组
    {
        for(i = 0; i < MinDegree; i++)
            LeftNode->Child[MinDegree + i] = RightNode->Child[i];
    }

    //取需合并的两个节点所夹的父节点中的关键字，放入合并后的节点中当作中间值
    LeftNode->Key[MinDegree - 1] = ParentNode->Key[LeftIndex];
    LeftNode->Num = MinDegree * 2 - 1;

    //删除父节点中被取走的关键字和子节点指针
    ShiftKey(ParentNode->Key, L, LeftIndex + 1, ParentNode->Num - 1);
    ShiftChild(ParentNode->Child, L, RightIndex + 1, ParentNode->Num);
    (ParentNode->Num)--;

    //若父节点是根节点，且合并后根节点为空，需要重新设定根节点
    if(ParentNode == T->Root && 0 == ParentNode->Num)
        T->Root = LeftNode;
}

//删除节点
//入口参数：T根节点，CurrentNode当前查找的节点，val需要删除的关键字
void BTDelete(PtrBT T, PtrBTNode CurrentNode, int Val)
{
    int Index;
    PtrBTNode Precursor, Successor, SubNode;

    Index = GetIndex(CurrentNode->Key, CurrentNode->Num, Val);

    if(Index < CurrentNode->Num && CurrentNode->Key[Index] == Val) //当前节点找到需要删除的关键字
    {
        if(True == CurrentNode->IsLeaf) //是叶节点，直接删除
        {
            ShiftKey(CurrentNode->Key, L, Index + 1, CurrentNode->Num - 1);
            (CurrentNode->Num)--;
            return;
        }
        else  //不是叶节点
        {
            Precursor = CurrentNode->Child[Index];
            Successor = CurrentNode->Child[Index + 1];

            if(Precursor->Num > MinDegree - 1) //若该关键字的前继节点中关键字关键字数量大于最低值，就从前继节点中取最大值覆盖该关键字，然后继续递归删除前继节点中的最大值
            {
                CurrentNode->Key[Index] = Precursor->Key[Precursor->Num - 1];
                BTDelete(T, Precursor, Precursor->Key[Precursor->Num - 1]);
            }

            else if(Successor->Num > MinDegree - 1)//若该关键字的后继节点中关键字关键字数量大于最低值，就从后继节点中取最小值覆盖该关键字，然后继续递归删除后继节点中的最小值
            {

                CurrentNode->Key[Index] = Successor->Key[0];
                BTDelete(T, Successor, Successor->Key[0]);
            }
            else//若该关键字的前继后继节点关键字数量均不大于最低值，需先进行前继和后继的合并，然后再进行递归删除
            {

                Merge(T, CurrentNode, Index, Index + 1);
                BTDelete(T, CurrentNode->Child[Index], Val);//合并后需要删除的节点就在合并的节点中
            }

        }

    }
    else//当前节点未找到需要删除的关键字
    { 

        if(True == CurrentNode->IsLeaf) //是叶节点，该树中没有需要删除的关键字
            return;
        else
        {
            SubNode = CurrentNode->Child[Index];
            if(SubNode->Num > MinDegree - 1) //下一个用于查找需要删除的关键字的节点中关键字数量大于最小值，继续在该节点进行递归删除
                BTDelete(T, SubNode, Val);
            else//下一个用于查找需要删除的关键字的节点中关键字数量小于等于最小值
            {
                //当下一个进行查找的节点有可能没有左兄弟节点（前继）或右兄弟节点（后继），因此需要进行判断
                if(Index > 0)
                    Precursor = CurrentNode->Child[Index - 1];
                if(Index < CurrentNode->Num)
                    Successor = CurrentNode->Child[Index + 1];

                if(Index > 0 && Precursor->Num > MinDegree - 1) //如果左兄弟节点中关键字数量大于最低值，可以进行向右旋转
                {

                    ShiftKey(SubNode->Key, R, 0, SubNode->Num - 1);
                    ShiftChild(SubNode->Child, R, 0, SubNode->Num);

                    SubNode->Key[0] = CurrentNode->Key[Index - 1];
                    SubNode->Child[0] = Precursor->Child[Precursor->Num];
                    CurrentNode->Key[Index - 1] = Precursor->Key[Precursor->Num - 1];
                    (SubNode->Num)++;
                    (Precursor->Num)--;
                    
                    BTDelete(T, SubNode, Val);
                }
                else if(Index < CurrentNode->Num && Successor->Num > MinDegree - 1)//如果右兄弟节点中关键字数量大于最低值，可以进行向左旋转
                {
                    SubNode->Key[SubNode->Num] = CurrentNode->Key[Index];
                    SubNode->Child[SubNode->Num + 1] = Successor->Child[0];
                    (SubNode->Num)++;
                    CurrentNode->Key[Index] = Successor->Key[0];

                    ShiftKey(Successor->Key, L, 1, Successor->Num - 1);
                    ShiftChild(Successor->Child, L, 1, Successor->Num);
                    (Successor->Num)--;

                    BTDelete(T, CurrentNode->Child[Index], Val);
                }
                else  //如果左右兄弟节点中关键字数量都不大于最低值，需要进行合并
                {
                    if(Index > 0) //有左兄弟，首先与左兄弟合并，没有再和右兄弟合并
                    {
                        Merge(T, CurrentNode, Index - 1, Index);
                        BTDelete(T, Precursor, Val);

                    }
                    else
                    {
                        Merge(T, CurrentNode, Index, Index + 1);
                        BTDelete(T, SubNode, Val);
                    }
                }

            }

        }
    }
}

//遍历输出树
//入口参数：Root树的根节点
void BTPrintTree(PtrBTNode Root)
{
    int i;

    if(NULL == Root)
        return;
    putchar('[');
    for(i = 0; i < Root->Num; i++)
    {
        printf("%d", Root->Key[i]);
        if(i != Root->Num - 1)
            putchar(' ');
    }
    putchar(']');
    printf("%d", Root->IsLeaf);
    putchar('\n');

    for(i = 0; i <= Root->Num; i++)
        BTPrintTree(Root->Child[i]);
}

//队列结构体
#define queuesize 100
typedef struct queue
{
	int head, tail; 
	PtrBTNode *Queue_BTNode;          
}Queue;

//初始化队列
void InitQueue(Queue *q)
{
	q->Queue_BTNode = (PtrBTNode*)malloc(sizeof(PtrBTNode) * queuesize);
	q->tail = 0;
	q->head = 0;
}

//入队
void EnQueue(Queue *q, PtrBTNode BTNode)
{
	int tail = (q->tail + 1) % queuesize; 
	if (tail == q->head)                   
	{
		printf("the queue has been filled full!");
	}
	else
	{
        q->Queue_BTNode[q->tail] = BTNode;
		q->tail = tail;
	}
}

//出队
PtrBTNode DeQueue(Queue *q)
{
	PtrBTNode tmp;
	if (q->tail == q->head)     
	{
		printf("the queue is NULL\n");
	}
	else
	{
		tmp = q->Queue_BTNode[q->head];
		q->head = (q->head + 1) % queuesize;
	}
	return tmp;
}

//层序遍历
void BTtraversal_level(PtrBTNode Root)
{
    Queue BTNode_Queue;
    PtrBTNode tmp;
    int i,j;

    if(NULL == Root)
        return;
    InitQueue(&BTNode_Queue);

    printf("[");
    for(j = 0; j < Root -> Num; j++)
    {
        printf("%d", Root -> Key[j]);
        if(j != Root->Num - 1)
        printf(" ");
    }
    printf("]%d ",Root->IsLeaf);
    if(Root->IsLeaf != True)
        EnQueue(&BTNode_Queue,Root);
    
    
    while(BTNode_Queue.head != BTNode_Queue.tail)
    {
        tmp = DeQueue(&BTNode_Queue);
        for(i = 0; i <= tmp->Num; i++)
        {
            printf("[");
            for(j = 0; j < tmp -> Child[i]->Num; j++)
            {
                printf("%d", tmp ->Child[i]->Key[j]);
                if(j != tmp->Child[i]->Num - 1)
                    printf(" ");
            }
            printf("]%d ",tmp ->Child[i]->IsLeaf);
            if(tmp ->Child[i]->IsLeaf != True)
                EnQueue(&BTNode_Queue,tmp ->Child[i]);
        }
    }
    printf("\n");
}
//创建树
//入口参数：T树的根目录
void BTCreateTree(PtrBT T)
{
    int i;
    int a[] = {12,1,9,2,0,11,7,19,4,15,18,5,14,13,10,16,6,3,8,17,20,21,23};

    for(i = 0; i < 23; i++)
    {
        BTInsert(T, a[i]);
        BTtraversal_level(T -> Root);
        printf("The End\n\n");
    }
    
    
}

int main(void)
{
    PtrBT T = (PtrBT)malloc(sizeof(struct Tree));

    T->Root = BTAllocateNode();

    BTCreateTree(T);
    //BTtraversal_level(T -> Root);

    /*

    printf("B_Tree after delete 16:\n");
    BTDelete(T, T->Root, 16);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 18:\n");
    BTDelete(T, T->Root, 18);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 20:\n");
    BTDelete(T, T->Root, 20);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 19:\n");
    BTDelete(T, T->Root, 19);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 0:\n");
    BTDelete(T, T->Root, 0);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 5:\n");
    BTDelete(T, T->Root, 5);
    BTPrintTree(T->Root);

    printf("B_Tree after delete 2:\n");
    BTDelete(T, T->Root, 2);
    BTPrintTree(T->Root);
    */

    return 0;
}
