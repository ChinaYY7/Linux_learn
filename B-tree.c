//gcc B-tree.c B+_Tree/Bplus_Tree.c -o ../bin/B-tree.out -I B+_Tree
#include "apue.h"
#include <time.h>
#include <sys/time.h> 
#include "Bplus_Tree.h"

int main(void)
{
    int Cmd,Insert_val,Delete_val,Btree_Delete_stat;
    int Insert_num,Node_num,Leaf_node_num,All_val_num,Leaf_val_num,Delete_num = 0;
    int i;
    double Complate_time;
    struct timeval start,finish;
    FILE *BT_File_fp;
    PtrBT T;
    PtrBTNode head;
    
    BTCreateTree(&T, &head);

    while(1)
    {
        printf("\n/***********************Menu*******************************/\n\n");
        printf("1.Insert a number\n");
        printf("2.Insert many random numbers\n");
        printf("3.Delete\n");
        printf("4.Traversal B-tree\n");
        printf("5.Traversal leaf\n");
        printf("6.Get_B+Tree_Info\n");
        printf("7.Clear\n");
        printf("8.Write_File\n");
        printf("9.Read_File\n");
        printf("10.Clear_tree\n");
        printf("11.Exit\n");
        printf("\n/**********************************************************/\n\n");
        printf("Input Cmd: ");
        scanf("%d",&Cmd);
        if(Cmd % 12 == Cmd)
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
                    Btree_Delete_stat = BTDelete(T, T->Root, Delete_val,&Delete_num);
                    if(Success == Btree_Delete_stat)
                    {
                        printf("\nDelete_result: %d\n",Delete_num);
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
                    Get_Bplus_Tree_Info(T -> Root,&Node_num,&Leaf_node_num,&All_val_num,&Leaf_val_num);
                    printf("Node_num: %d, Leaf_node_num: %d, All_val_num: %d, Leaf_val_num: %d\n", Node_num,Leaf_node_num,All_val_num,Leaf_val_num);
                    break;
                case 7:
                    system("clear");
                    break;
                case 8:
                    if((BT_File_fp = fopen(BT_File_path,"w")) == NULL)
                        Error_Exit("Creat target file error\n");
                    Write_BTree(BT_File_fp, T -> Root);
                    fclose(BT_File_fp);
                    break;
                case 9:
                    if((BT_File_fp = fopen(BT_File_path,"r")) == NULL)
                        Error_Exit("Can not open file\n");
                    Read_BTree(BT_File_fp, T -> Root, &head);
                    fclose(BT_File_fp);
                    break;
                case 10:
                    Delete_num = 0;
                    Clear_tree(T,head,&Delete_num);
                    printf("\nDelete_result:%d\n",Delete_num);
                    BTtraversal_level(T -> Root);
                    BTtraversal_leaf(head);
                    break;
                case 11:
                    return 0;
            }
        }
        else
            printf("Cmd is wrong,please input again !\n");
    }
}
