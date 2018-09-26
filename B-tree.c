#include "apue.h"
#include <time.h>
#include <sys/time.h> 
#define R 1
#define L 0
#define Success 1
#define Fail 0
#define MinDegree 3
#define BT_File_path "/mnt/f/Linux_code/tmp/BT_File.txt"
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
    PtrBTNode next;
    PtrBTNode prior;
    int* date;
};
struct Tree{
    PtrBTNode Root;
};
typedef struct Tree *PtrBT;  //用于修改根节点时

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
    NewNode->prior = NULL;
    NewNode->next = NULL;
    
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

    if(True == FullNode->IsLeaf)
    {
        for(i = 0; i < MinDegree; i++)
            NewNode->Key[i] = FullNode->Key[MinDegree - 1 + i];
        NewNode->Num = MinDegree;
        FullNode->Num = MinDegree - 1;
        
        NewNode->next = FullNode->next;
        if(FullNode->next != NULL)
            FullNode->next->prior = NewNode;
        FullNode->next = NewNode;
        NewNode->prior = FullNode;
        

    }
    else
    {
        for(i = 0; i < MinDegree - 1; i++)
            NewNode->Key[i] = FullNode->Key[MinDegree + i];

        NewNode->IsLeaf = False;
        for(i = 0; i < MinDegree; i++)
            NewNode->Child[i] = FullNode->Child[MinDegree + i];
        NewNode->Num = FullNode->Num = MinDegree - 1;
    }
    

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

    for(i = 0; i < Size && Val >= Key[i]; i++);

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
void BTInsert(PtrBT T, int Val)
{

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

    if(True == LeftNode->IsLeaf)
    {
        for(i = 0; i < MinDegree - 1; i++)
            LeftNode->Key[MinDegree - 1 + i] = RightNode->Key[i];
        LeftNode->Num = MinDegree * 2 - 2;
        
        if(LeftNode->next->next != NULL)
            LeftNode->next->next->prior = LeftNode;
        LeftNode->next = LeftNode->next->next;
    }

    else
    {
        for(i = 0; i < MinDegree - 1; i++)
            LeftNode->Key[MinDegree + i] = RightNode->Key[i];
        for(i = 0; i < MinDegree; i++)
            LeftNode->Child[MinDegree + i] = RightNode->Child[i];

        //取需合并的两个节点所夹的父节点中的关键字，放入合并后的节点中当作中间值
        LeftNode->Key[MinDegree - 1] = ParentNode->Key[LeftIndex];
        LeftNode->Num = MinDegree * 2 - 1;
    }
    //删除父节点中被取走的关键字和子节点指针
    ShiftKey(ParentNode->Key, L, LeftIndex + 1, ParentNode->Num - 1);
    ShiftChild(ParentNode->Child, L, RightIndex + 1, ParentNode->Num);
    (ParentNode->Num)--;

    //若父节点是根节点，且合并后根节点为空，需要重新设定根节点
    if(ParentNode == T->Root && 0 == ParentNode->Num)
        T->Root = LeftNode;
    free(RightNode);
}

//删除节点
//入口参数：T根节点，CurrentNode当前查找的节点，val需要删除的关键字
int BTDelete(PtrBT T, PtrBTNode CurrentNode, int Val)
{
    int Index;
    PtrBTNode Precursor, Successor, SubNode;

    Index = GetIndex(CurrentNode->Key, CurrentNode->Num, Val);

    if(True == CurrentNode->IsLeaf) //是叶节点
    {
        if(CurrentNode->Key[Index - 1] == Val) //在叶节点中找到val，直接删除
        {
            ShiftKey(CurrentNode->Key, L, Index, CurrentNode->Num - 1);
            (CurrentNode->Num)--;
            return Success;
        }
        else
            return Fail;
    }
    else//不是叶节点
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

                if(True == SubNode->IsLeaf)
                {
                    SubNode->Key[0] = Precursor->Key[Precursor->Num - 1];
                    CurrentNode->Key[Index - 1] = Precursor->Key[Precursor->Num - 1];
                    (SubNode->Num)++;
                    (Precursor->Num)--;
                }
                else
                {
                    SubNode->Key[0] = CurrentNode->Key[Index - 1];
                    SubNode->Child[0] = Precursor->Child[Precursor->Num];
                    CurrentNode->Key[Index - 1] = Precursor->Key[Precursor->Num - 1];
                    (SubNode->Num)++;
                    (Precursor->Num)--;
                }
                BTDelete(T, SubNode, Val);
            }
            else if(Index < CurrentNode->Num && Successor->Num > MinDegree - 1)//如果右兄弟节点中关键字数量大于最低值，可以进行向左旋转
            {
                if(True == SubNode->IsLeaf)
                {
                    SubNode->Key[SubNode->Num] = CurrentNode->Key[Index];
                    (SubNode->Num)++;
                    CurrentNode->Key[Index] = Successor->Key[1];
                    ShiftKey(Successor->Key, L, 1, Successor->Num - 1);
                    (Successor->Num)--;
                }
                else
                {
                    SubNode->Key[SubNode->Num] = CurrentNode->Key[Index];
                    SubNode->Child[SubNode->Num + 1] = Successor->Child[0];
                    (SubNode->Num)++;
                    CurrentNode->Key[Index] = Successor->Key[0];

                    ShiftKey(Successor->Key, L, 1, Successor->Num - 1);
                    ShiftChild(Successor->Child, L, 1, Successor->Num);
                    (Successor->Num)--;
                }
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
        //return Fail;
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
void DeleteQueue(Queue *q)
{
    free(q -> Queue_BTNode);
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
        printf("\n");
    }
    printf("\n\n");
    DeleteQueue(&BTNode_Queue);
}

//叶节点链表遍历
void BTtraversal_leaf(PtrBTNode head)
{
    PtrBTNode Ptree;
    int i;
    Ptree = head->next;
    
    while(Ptree != NULL)
    {
        
        putchar('[');
        
        for(i = 0; i < Ptree->Num; i++)
        {
            printf("%d", Ptree->Key[i]);
            if(i != Ptree->Num - 1)
                putchar(' ');
        }
        putchar(']');
        printf("%d ", Ptree->IsLeaf);
        Ptree = Ptree->next;
    }
    printf("\n");
}

//清空树
void Clear_tree(PtrBT T, PtrBTNode head)
{
    PtrBTNode Ptree;
    int i;
    Ptree = head->next;
    while(Ptree->Num != 0)
    {
        BTDelete(T, T->Root, Ptree->Key[0]);
    }
}
//创建树
//入口参数：T树的根目录
void BTCreateTree(PtrBT T)
{
    int i;
    int a[] = {12,1,9,2,0,11,7,19,4,15,18,5,14,13,10,16,6,3,8,17,20,21,23};

    for(i = 0; i < 23; i++)
    {
        //printf("Insert %d\n",a[i]);
        BTInsert(T, a[i]);
        //BTtraversal_level(T -> Root);
    }
}

//出错处理
void Error_Exit(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}
//写入节点结构体
typedef struct Node_simple
{
	int key[MinDegree * 2 - 1];
    int Num;
    Bool IsLeaf;         
}Write_Node, Read_Node;

//写入节点
void Write_BTNode(FILE *BT_File_fp, PtrBTNode BTNode)
{
    Write_Node Node;
    char i;
    for (i = 0; i < BTNode->Num; i++)
        Node.key[i] = BTNode->Key[i];
    Node.Num = BTNode->Num;
    Node.IsLeaf = BTNode->IsLeaf;
    //printf("Node.Num = %d, Node.IsLeaf = %d\n", Node.Num,Node.IsLeaf);
    fwrite(&Node,sizeof(struct Node_simple),1,BT_File_fp);
}

//写入树
void Write_BTree(FILE *BT_File_fp, PtrBTNode Root)
{
    Queue BTNode_Queue;
    PtrBTNode tmp;
    int i;

    if(NULL == Root)
        return;
    InitQueue(&BTNode_Queue);
    Write_BTNode(BT_File_fp,Root);
    if(Root->IsLeaf != True)
        EnQueue(&BTNode_Queue,Root);

    while(BTNode_Queue.head != BTNode_Queue.tail)
    {
        tmp = DeQueue(&BTNode_Queue);
        for(i = 0; i <= tmp->Num; i++)
        {
            Write_BTNode(BT_File_fp,tmp->Child[i]);
            if(tmp ->Child[i]->IsLeaf != True)
                EnQueue(&BTNode_Queue,tmp ->Child[i]);
        }
    }
    DeleteQueue(&BTNode_Queue);
}

//读入节点
void Read_BTNode(FILE *BT_File_fp, PtrBTNode BTNode)
{
    int i;
    Read_Node Node;
    fread(&Node,sizeof(struct Node_simple),1,BT_File_fp);
    
    BTNode->Num = Node.Num;
    BTNode->IsLeaf = Node.IsLeaf;
    //printf("%d %d \n",BTNode->Num,BTNode->IsLeaf);
    for(i = 0; i < BTNode->Num; i++)
        BTNode->Key[i] = Node.key[i];
}

//读入树
void Read_BTree(FILE *BT_File_fp, PtrBTNode Root, PtrBTNode *head)
{
    Queue BTNode_Queue;
    PtrBTNode tmp,NewNode,Node_p;
    int First_Lnode = 1;
    int i;

    if(NULL == Root)
        return;
    InitQueue(&BTNode_Queue);
    Read_BTNode(BT_File_fp,Root);
    if(Root->IsLeaf != True)
        EnQueue(&BTNode_Queue,Root);
    while(BTNode_Queue.head != BTNode_Queue.tail)
    {
        tmp = DeQueue(&BTNode_Queue);
        for(i = 0; i <= tmp -> Num; i++)
        {
            NewNode = BTAllocateNode();
            Read_BTNode(BT_File_fp,NewNode);
            if(NewNode->IsLeaf != 1)
                EnQueue(&BTNode_Queue,NewNode);
            else
            {
                if(First_Lnode)
                {
                    First_Lnode = 0;
                    (*head)->next = NewNode;
                    NewNode->prior = *head;
                    Node_p = NewNode;
                }
                Node_p->next = NewNode;
                NewNode->prior = Node_p;
                Node_p = NewNode;
            }
            tmp->Child[i] = NewNode;
        }
        NewNode->next = NULL;
    }
    DeleteQueue(&BTNode_Queue);
}


int main(void)
{
    int Cmd,Insert_val,Delete_val,Btree_Delete_stat;
    int Insert_num;
    int i;
    double Complate_time;
    struct timeval start,finish;
    FILE *BT_File_fp;

    PtrBT T = (PtrBT)malloc(sizeof(struct Tree));
    PtrBTNode head;
    head = BTAllocateNode();
    T->Root = BTAllocateNode();
    
    head->next = T->Root;
    T->Root->prior = head->next;
    T->Root->next = NULL;

    //BTCreateTree(T);

    while(1)
    {
        printf("\n/***********************Menu*******************************/\n\n");
        printf("1.Insert a number\n");
        printf("2.Insert many random numbers\n");
        printf("3.Delete\n");
        printf("4.Traversal B-tree\n");
        printf("5.Traversal leaf\n");
        printf("6.Clear\n");
        printf("7.Write_File\n");
        printf("8.Read_File\n");
        printf("9.Clear_tree\n");
        printf("10.Exit\n");
        printf("\n/**********************************************************/\n\n");
        printf("Input Cmd: ");
        scanf("%d",&Cmd);
        if(Cmd % 11 == Cmd)
        {
            switch(Cmd)
            {
                case 1:
                    //system("clear");
                    printf("Input insert val: ");
                    scanf("%d",&Insert_val);
                    BTInsert(T, Insert_val);

                    printf("\nSequence traversal:\n");
                    BTtraversal_level(T -> Root);

                    printf("\nList traversal:\n");
                    BTtraversal_leaf(head);
                    
                    break;
                case 2:
                    //srand(time(NULL));
                    printf("Input the number of inserted Numbers:");
                    scanf("%d",&Insert_num);
                    gettimeofday(&start,NULL);//获取程序开始时间
                    for(i = 0; i < Insert_num; i++)
                        BTInsert(T, i);
                        //BTInsert(T, rand() % 10000000);
                    
                    gettimeofday(&finish,NULL);
                    Complate_time = (double)((finish.tv_sec-start.tv_sec) * 1000000 + (finish.tv_usec-start.tv_usec)) / 1000000;
                    printf("\nInsert complated !\ntime: %.3f s\n",Complate_time);
                    break;
                case 3:
                    //system("clear");
                    printf("Input delete val: ");
                    scanf("%d",&Delete_val);
                    Btree_Delete_stat = BTDelete(T, T->Root, Delete_val);
                    if(Success == Btree_Delete_stat)
                    {
                        printf("\nDelete_result:\n");
                        BTtraversal_level(T -> Root);
                    }
                    else
                        printf("Not found val:%d\n",Delete_val);
                    break;
                case 4:
                    //system("clear");
                    printf("\nSequence traversal:\n");
                    BTtraversal_level(T -> Root);
                    break;
                case 5:
                    printf("\nList traversal:\n");
                    BTtraversal_leaf(head);
                    break;
                case 6:
                    system("clear");
                    break;
                case 7:
                    if((BT_File_fp = fopen(BT_File_path,"w")) == NULL)
                        Error_Exit("Creat target file error\n");
                    Write_BTree(BT_File_fp, T -> Root);
                    fclose(BT_File_fp);
                    break;
                case 8:
                    if((BT_File_fp = fopen(BT_File_path,"r")) == NULL)
                        Error_Exit("Can not open file\n");
                    Read_BTree(BT_File_fp, T -> Root, &head);
                    fclose(BT_File_fp);
                    break;
                case 9:
                    Clear_tree(T,head);
                    BTtraversal_level(T -> Root);
                    BTtraversal_leaf(head);
                    break;
                case 10:
                    return 0;
            }
        }
        else
            printf("Cmd is wrong,please input again !\n");
    }
}
