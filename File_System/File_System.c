#include "File_System.h"
#include <time.h>

static FILE *File_System_fp;
static Surper_Block Surper_Block_Info;
static Mem_Inode_P Mem_Inode_head;
static int Indirect_Block_Quantity = BLOCK_SIZE / sizeof(int);
static int Current_Dirent_Inode_Num;

//块号转文件偏移量
//入口参数：块号
//返回值：文件偏移量
int Block_Num_to_Offset(int Block_Num)
{
    unsigned int Offset;
    Offset = Surper_Block_Info.Start_Block_Offset + Block_Num * BLOCK_SIZE;
    return Offset;
}

//结点号转文件偏移量
//入口参数：结点号
//返回值：文件偏移量
int Inode_Num_to_Offset(int Inode_Num)
{
    unsigned int Offset;
    int Disk_Inode_Size = sizeof(Disk_Inode);
    Offset = Surper_Block_Info.Start_Inode_Offset + Inode_Num * Disk_Inode_Size;
    return Offset;
}

//读取文件块里所有int数据
//入口参数：Block_Num：块号； Block_Buf：整型数组；
//返回值：0：正常： 1：读取失败
int Read_Block_Int_Full(int Block_Num, int *Block_Buf)
{
    unsigned int Offset;
    int Read_Num;

    Read_Num = BLOCK_SIZE / sizeof(int);
    
    Offset = Block_Num_to_Offset(Block_Num);
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fread(Block_Buf, sizeof(int), Read_Num, File_System_fp) != Read_Num)
        return 1;
    return 0;
}

//从文件块中读取任意一个整型
//入口参数：Block_Num：块号；  One_Int：整型值；  Int_num：整型号；
//返回值：0：正常； 1：读取整型失败；
int Read_Block_Int_One(int Block_Num, int *One_Int, int Int_num)
{
    unsigned int Offset;

    Offset = Block_Num_to_Offset(Block_Num) + Int_num * sizeof(int);
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fread(One_Int, sizeof(int), 1, File_System_fp) != 1)
        return 1;
    return 0;
}

//往块中写入int整型
//入口参数：Block_Num：块号， Block_Buf：要写入的int数组， Write_Num：写入的数量
//返回值：0：成功， 1：写入失败
int Write_Block_Int(int Block_Num, int *Block_Buf, int Write_Num)
{
    unsigned int Offset;

    Offset = Block_Num_to_Offset(Block_Num);
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fwrite(Block_Buf, sizeof(int), Write_Num, File_System_fp) != Write_Num)
        return 1;
    return 0;
}

int Write_Block_Int_One(int Block_Num, int *One_Int, int Int_num)
{
    unsigned int Offset;

    Offset = Block_Num_to_Offset(Block_Num) + Int_num * sizeof(int);
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fwrite(One_Int, sizeof(int), 1, File_System_fp) != 1)
        return 1;
    return 0;
}

int Balloc(void)
{
    int Block_Num;
    
    if(Surper_Block_Info.Free_Block_Num == 0)
        return -1;
    Block_Num = Surper_Block_Info.Free_Block_Table[Surper_Block_Info.Free_Block_Suffix];
    Surper_Block_Info.Free_Block_Suffix++;
    if(Surper_Block_Info.Free_Block_Suffix - 1 == FREE_BLOCK_TABLE_SIZE - 1)
    {
        if(Read_Block_Int_Full(Block_Num, Surper_Block_Info.Free_Block_Table) == 1)
            return -2;
        Surper_Block_Info.Free_Block_Suffix = 0;
    }
    Surper_Block_Info.Free_Block_Num--;
    Surper_Block_Info.Modified = True;
    return Block_Num;
}

//释放文件块
//入口参数：块号
//返回值：0：正常； 1：写入整型失败
int Bfree(int Block_Num)
{
    if(Surper_Block_Info.Free_Inode_Suffix == 0) //超级块空闲块表已满
    {
        if(Write_Block_Int(Block_Num,Surper_Block_Info.Free_Block_Table,Indirect_Block_Quantity) != 0)
            return 1;
        Surper_Block_Info.Free_Inode_Suffix = FREE_BLOCK_TABLE_SIZE - 1;
        Surper_Block_Info.Free_Block_Table[Surper_Block_Info.Free_Inode_Suffix] = Block_Num;
    }
    else
    {
        Surper_Block_Info.Free_Inode_Suffix--;
        Surper_Block_Info.Free_Block_Table[Surper_Block_Info.Free_Inode_Suffix] = Block_Num;
    }
    Surper_Block_Info.Free_Block_Num++;
    Surper_Block_Info.Modified = True;
    return 0;
}

//释放文件块
//入口参数：内存结点指针
//返回值：0：正常； 1：读取文件块失败
int Bfree_Inode(Mem_Inode_P Mem_Inode_Info)
{
    int i,j,m,n;
    int Free_Block_Num1,Free_Block_Num2,Free_Block_Num3;

    for(i = 0; i < 13; i++)
    {
        if(Mem_Inode_Info->File_Disk_Address[i] != -1)
        {
            if(i > 9)
            {
                for(j = 0; j < Indirect_Block_Quantity; j++)
                {
                    if(Read_Block_Int_One(Mem_Inode_Info->File_Disk_Address[i],&Free_Block_Num1,j) != 0)
                        return 1;
                    if(Free_Block_Num1 != -1)
                    {
                        if(i > 10)
                        {
                            for(m = 0; m < Indirect_Block_Quantity; m++)
                            {
                                if(Read_Block_Int_One(Free_Block_Num1,&Free_Block_Num2,m) != 0)
                                    return 1;
                                if(Free_Block_Num2 != -1)
                                {
                                    if(i > 11)
                                    {
                                        for(n = 0; n < Indirect_Block_Quantity; n++)
                                        {
                                            if(Read_Block_Int_One(Free_Block_Num2,&Free_Block_Num3,n) != 0)
                                                return 1;
                                            if(Free_Block_Num3 != -1)
                                                if(Bfree(Free_Block_Num3) != 0)
                                                    return 1;
                                        }
                                    }
                                    if(Bfree(Free_Block_Num2) != 0)
                                        return 1;
                                    Free_Block_Num2 = -1;
                                }
                            }
                        }
                        if(Bfree(Free_Block_Num1) != 0)
                            return 1;
                        Free_Block_Num1 = -1;
                    }
                }
            }
            if(Bfree(Mem_Inode_Info->File_Disk_Address[i]) != 0)
                return 1;
            Mem_Inode_Info->File_Disk_Address[i] = -1;
        }
    } 
    return 0;        
}

int Index_Block_Init(int Disk_Block_Num)
{
    int i;
    int Block_Data[Indirect_Block_Quantity];
    for(i = 0; i < Indirect_Block_Quantity; i++)
        Block_Data[i] = INDEX_BLOCK_FREE;
    if(Write_Block_Int(Disk_Block_Num,Block_Data,Indirect_Block_Quantity) != 0)
        return 1;
    return 0;
}

int Bmap_Direct_Block(int Block_Num, int Block_Offset,Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta)
{
    if(Block_Num > 9)
        return 1;
    if(Sta == True)
    {
        if(Mem_Inode_Num->File_Disk_Address[Block_Num] == -1)
        {
            Mem_Inode_Num->File_Disk_Address[Block_Num] = Balloc();
            if(Mem_Inode_Num->File_Disk_Address[Block_Num] < 0)
                return 2;
        }
    }
    if(Mem_Inode_Num->File_Disk_Address[Block_Num] == -1)
        return 3;
    *Offset = Block_Num_to_Offset(Mem_Inode_Num->File_Disk_Address[Block_Num]) + Block_Offset;
    return 0;
}



int Bmap_Indirect1_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta)
{
    int Indirect1_Block_Num;

    Block_Num  = Block_Num - 10;
    if(Block_Num >= Indirect_Block_Quantity)
        return 1;
    if(Sta == True)
    {
        if(Mem_Inode_Num->File_Disk_Address[10] == INDEX_BLOCK_FREE)
        {
            Mem_Inode_Num->File_Disk_Address[10] = Balloc();
            if(Mem_Inode_Num->File_Disk_Address[10] < 0)
                return 2;
            if(Index_Block_Init(Mem_Inode_Num->File_Disk_Address[10]) != 0)
                return 3;
        }
        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[10], &Indirect1_Block_Num, Block_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect1_Block_Num = Balloc();
            if(Indirect1_Block_Num < 0)
                return 2;
            if(Write_Block_Int_One(Mem_Inode_Num->File_Disk_Address[10], &Indirect1_Block_Num, Block_Num) != 0)
                return 3;
        }    
    }
    else
    {
        if(Mem_Inode_Num->File_Disk_Address[10] == INDEX_BLOCK_FREE)
                return 4;
        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[10], &Indirect1_Block_Num, Block_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
            return 4;
    }
    *Offset = Block_Num_to_Offset(Indirect1_Block_Num) + Block_Offset;
    return 0;
}

int Bmap_Indirect2_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta)
{
    int Block1_Num, Block2_Num;
    int Indirect1_Block_Num, Indirect2_Block_Num;
    
    Block_Num  = Block_Num - 10 - Indirect_Block_Quantity;
    if(Block_Num >= Indirect_Block_Quantity * Indirect_Block_Quantity)
        return 1;

    Block1_Num = Block_Num / Indirect_Block_Quantity;
    Block2_Num = Block_Num % Indirect_Block_Quantity;
    
    if(Sta == True)
    {
        if(Mem_Inode_Num->File_Disk_Address[11] == INDEX_BLOCK_FREE)
        {
            Mem_Inode_Num->File_Disk_Address[11] = Balloc();
            if(Mem_Inode_Num->File_Disk_Address[11] < 0)
                return 2;
            if(Index_Block_Init(Mem_Inode_Num->File_Disk_Address[11]) != 0)
                return 3;
        }

        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[11], &Indirect1_Block_Num, Block1_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect1_Block_Num = Balloc();
            if(Indirect1_Block_Num < 0)
                return 2;
            if(Index_Block_Init(Indirect1_Block_Num) != 0)
                return 3;
            if(Write_Block_Int_One(Mem_Inode_Num->File_Disk_Address[11], &Indirect1_Block_Num, Block1_Num) != 0)
                return 3;
        }

        if(Read_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
            return 3;
        if(Indirect2_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect2_Block_Num = Balloc();
            if(Indirect2_Block_Num < 0)
                return 2;
            if(Write_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
                return 3;
        }
        
    }
    else
    {
        if(Mem_Inode_Num->File_Disk_Address[11] == INDEX_BLOCK_FREE)
                return 4;

        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[11], &Indirect1_Block_Num, Block1_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
            return 4;
        
        if(Read_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
            return 3;
        if(Indirect2_Block_Num == INDEX_BLOCK_FREE)
            return 4;
    }
    *Offset = Block_Num_to_Offset(Indirect2_Block_Num) + Block_Offset;
    return 0;
}

int Bmap_Indirect3_Block(int Block_Num, int Block_Offset, Mem_Inode_P Mem_Inode_Num,int *Offset, Bool Sta)
{
    int Block1_Num, Block2_Num, Block3_Num;
    int Indirect1_Block_Num, Indirect2_Block_Num, Indirect3_Block_Num;
    
    Block_Num  = Block_Num - 10 - (Indirect_Block_Quantity * Indirect_Block_Quantity);
    if(Block_Num >= Indirect_Block_Quantity * Indirect_Block_Quantity * Indirect_Block_Quantity)
        return 1;

    Block1_Num = Block_Num / Indirect_Block_Quantity / Indirect_Block_Quantity;
    Block2_Num = Block_Num / Indirect_Block_Quantity % Indirect_Block_Quantity;
    Block3_Num = Block_Num % Indirect_Block_Quantity;
    
    if(Sta == True)
    {
        if(Mem_Inode_Num->File_Disk_Address[12] == INDEX_BLOCK_FREE)
        {
            Mem_Inode_Num->File_Disk_Address[12] = Balloc();
            if(Mem_Inode_Num->File_Disk_Address[12] < 0)
                return 2;
            if(Index_Block_Init(Mem_Inode_Num->File_Disk_Address[12]) != 0)
                return 3;
        }

        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[12], &Indirect1_Block_Num, Block1_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect1_Block_Num = Balloc();
            if(Indirect1_Block_Num < 0)
                return 2;
            if(Index_Block_Init(Indirect1_Block_Num) != 0)
                return 3;
            if(Write_Block_Int_One(Mem_Inode_Num->File_Disk_Address[11], &Indirect1_Block_Num, Block1_Num) != 0)
                return 3;
        }

        if(Read_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
            return 3;
        if(Indirect2_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect2_Block_Num = Balloc();
            if(Indirect2_Block_Num < 0)
                return 2;
            if(Index_Block_Init(Indirect2_Block_Num) != 0)
                return 3;
            if(Write_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
                return 3;
        }
        
        if(Read_Block_Int_One(Indirect2_Block_Num, &Indirect3_Block_Num, Block3_Num) != 0)
            return 3;
        if(Indirect3_Block_Num == INDEX_BLOCK_FREE)
        {
            Indirect3_Block_Num = Balloc();
            if(Indirect3_Block_Num < 0)
                return 2;
            if(Write_Block_Int_One(Indirect2_Block_Num, &Indirect3_Block_Num, Block3_Num) != 0)
                return 3;
        }
    }
    else
    {
        if(Mem_Inode_Num->File_Disk_Address[11] == INDEX_BLOCK_FREE)
                return 4;

        if(Read_Block_Int_One(Mem_Inode_Num->File_Disk_Address[11], &Indirect1_Block_Num, Block1_Num) != 0)
            return 3;
        if(Indirect1_Block_Num == INDEX_BLOCK_FREE)
            return 4;
        
        if(Read_Block_Int_One(Indirect1_Block_Num, &Indirect2_Block_Num, Block2_Num) != 0)
            return 3;
        if(Indirect2_Block_Num == INDEX_BLOCK_FREE)
            return 4;

        if(Read_Block_Int_One(Indirect2_Block_Num, &Indirect3_Block_Num, Block3_Num) != 0)
            return 3;
        if(Indirect2_Block_Num == INDEX_BLOCK_FREE)
            return 4;
    }
    *Offset = Block_Num_to_Offset(Indirect3_Block_Num) + Block_Offset;
    return 0;
}

int File_Bmap(int Address, Mem_Inode_P Mem_Inode_Num, int *Offset, int *Useful_Size, Bool Sta)
{
    int Block_Num,Block_Offset;
    int Block1_Quantity,Block2_Quantity,Block3_Quantity;

    Block_Num = Address / BLOCK_SIZE;
    Block_Offset = Address % BLOCK_SIZE;
    *Useful_Size = BLOCK_SIZE - Block_Offset;

    Block1_Quantity = Indirect_Block_Quantity;
    Block2_Quantity = Block1_Quantity * Block1_Quantity;
    Block3_Quantity = Block2_Quantity * Block1_Quantity;

    if(Block_Num < 10)
    {
        if(Bmap_Direct_Block(Block_Num,Block_Offset,Mem_Inode_Num,Offset,Sta) != 0)
            return 1;
    }
    else if(Block_Num < 10 + Block1_Quantity)
    {
        if(Bmap_Indirect1_Block(Block_Num,Block_Offset,Mem_Inode_Num,Offset,Sta) != 0)
            return 1;
    }
    else if(Block_Num < 10 + Block1_Quantity + Block2_Quantity)
    {
        if(Bmap_Indirect2_Block(Block_Num,Block_Offset,Mem_Inode_Num,Offset,Sta) != 0)
            return 1;
    }
    else if(Block_Num < 10 + Block1_Quantity + Block2_Quantity + Block3_Quantity)
    {
        if(Bmap_Indirect3_Block(Block_Num,Block_Offset,Mem_Inode_Num,Offset,Sta) != 0)
            return 1;
    }
    return 0;
}

int Bmap_Block(int Address, Mem_Inode_P Mem_Inode_Num, int *Offset, int *Useful_Size, Bool Sta)
{
    int i;
    int Block_Num,Block_Offset,Block_In_Num[3];
    int Block1_Quantity,Block2_Quantity,Block3_Quantity;
    int Operate_Block_Num,Indirect_Block_Num,Indirect_Level, File_Disk_Address_Suffix;

    Block_Num = Address / BLOCK_SIZE;
    Block_Offset = Address % BLOCK_SIZE;
    *Useful_Size = BLOCK_SIZE - Block_Offset;

    Block1_Quantity = Indirect_Block_Quantity;
    Block2_Quantity = Block1_Quantity * Block1_Quantity;
    Block3_Quantity = Block2_Quantity * Block1_Quantity;

    if(Block_Num < 10)
    {
        Indirect_Level = 0;
        File_Disk_Address_Suffix = Block_Num;
    }
    else if (Block_Num < 10 + Block1_Quantity)
    {
        Indirect_Level = 1;
        Block_In_Num[0] = Block_Num - 10;
        File_Disk_Address_Suffix = 10;
    }
    else if (Block_Num < 10 + Block1_Quantity + Block2_Quantity)
    {
        Indirect_Level = 2;
        Block_Num = Block_Num - 10 - Indirect_Block_Quantity;
        Block_In_Num[0] = Block_Num / Indirect_Block_Quantity;
        Block_In_Num[1] = Block_Num % Indirect_Block_Quantity;
        File_Disk_Address_Suffix = 11;
    }
    else if (Block_Num < 10 + Block1_Quantity + Block2_Quantity + Block3_Quantity)
    {
        Indirect_Level = 3;
        Block_Num  = Block_Num - 10 - (Indirect_Block_Quantity * Indirect_Block_Quantity);
        Block_In_Num[0] = Block_Num / Indirect_Block_Quantity / Indirect_Block_Quantity;
        Block_In_Num[1] = Block_Num / Indirect_Block_Quantity % Indirect_Block_Quantity;
        Block_In_Num[2] = Block_Num % Indirect_Block_Quantity;
        File_Disk_Address_Suffix = 12;
    }

    if(Mem_Inode_Num->File_Disk_Address[File_Disk_Address_Suffix] == -1)
    {
        if(Sta == True)
        {
            Mem_Inode_Num->File_Disk_Address[File_Disk_Address_Suffix] = Balloc();
            if(Mem_Inode_Num->File_Disk_Address[File_Disk_Address_Suffix] < 0)
                return 2;
            if(Indirect_Level > 0)
                if(Index_Block_Init(Mem_Inode_Num->File_Disk_Address[File_Disk_Address_Suffix]) != 0)
                    return 3;
        }
        else
            return 1;
    }
    Operate_Block_Num = Mem_Inode_Num->File_Disk_Address[File_Disk_Address_Suffix];

    for(i = 0; i < Indirect_Level; i++)
    {
        if(Read_Block_Int_One(Operate_Block_Num, &Indirect_Block_Num, Block_In_Num[i]) != 0)
            return 3;
        if(Indirect_Block_Num == INDEX_BLOCK_FREE)
        {
            if(Sta == True)
            {
                Indirect_Block_Num = Balloc();
                if(Indirect_Block_Num < 0)
                    return 2;
                if(Write_Block_Int_One(Operate_Block_Num, &Indirect_Block_Num, Block_In_Num[i]) != 0)
                    return 3;
                if(i + 1 < Indirect_Level)
                    if(Index_Block_Init(Indirect_Block_Num) != 0)
                    return 3;
            }
            else
                return 1;
        }
        Operate_Block_Num = Indirect_Block_Num;   
    }

    *Offset = Block_Num_to_Offset(Operate_Block_Num) + Block_Offset;

    return 0;
}

//将结点信息读入内存结点并初始化内存结点信息
//入口参数：Inode_Num：结点号； Mem_Inode_Info：内存结点指针；
//返回值：0：正常； 1：读取结点失败
int Read_Inode(int Inode_Num, Mem_Inode_P Mem_Inode_Info)
{
    unsigned int Offset;
    int ret,i;
    Disk_Inode tmp;

    Offset = Inode_Num_to_Offset(Inode_Num);
    fseek(File_System_fp,Offset,SEEK_SET);
    if((ret = fread(&tmp, sizeof(Disk_Inode), 1, File_System_fp)) != 1)
        return 1;

    strcpy(Mem_Inode_Info->File_Access_Time,tmp.File_Access_Time);
    strcpy(Mem_Inode_Info->File_Modifie_Time,tmp.File_Modifie_Time);
    strcpy(Mem_Inode_Info->Inode_Modifie_Time,tmp.Inode_Modifie_Time);
    strcpy(Mem_Inode_Info->File_Creat_Time,tmp.File_Creat_Time);
    Mem_Inode_Info->File_Name_Link = tmp.File_Name_Link;
    Mem_Inode_Info->File_Size = tmp.File_Size;
    Mem_Inode_Info->File_Type = tmp.File_Type;
    for(i = 0; i < 13; i++)
        Mem_Inode_Info->File_Disk_Address[i] = tmp.File_Disk_Address[i];
        
    Mem_Inode_Info->Modified_Inode = False;
    return 0;
}

//将内存结点写入文件结点中
//入口参数：内存结点指针
//返回值：0：正常； 1：写入失败
int Write_Inode(Mem_Inode_P Mem_Inode_Info)
{
    unsigned int Offset;
    int ret,i;
    Disk_Inode tmp;

    strcpy(tmp.File_Access_Time,Mem_Inode_Info->File_Access_Time);
    strcpy(tmp.File_Modifie_Time,Mem_Inode_Info->File_Modifie_Time);
    strcpy(tmp.File_Creat_Time,Mem_Inode_Info->File_Creat_Time);
    strcpy(tmp.Inode_Modifie_Time,Mem_Inode_Info->Inode_Modifie_Time);
    tmp.File_Name_Link = Mem_Inode_Info->File_Name_Link;
    tmp.File_Size = Mem_Inode_Info->File_Size;
    tmp.File_Type = Mem_Inode_Info->File_Type;
    for(i = 0; i < 13; i++)
        tmp.File_Disk_Address[i] = Mem_Inode_Info->File_Disk_Address[i];
    
    Offset = Inode_Num_to_Offset(Mem_Inode_Info->Inode_Num);
    fseek(File_System_fp,Offset,SEEK_SET);
    if((ret = fwrite(&tmp, sizeof(Disk_Inode), 1, File_System_fp)) != 1)
        return 1;
    return 0;
}

//更新结点，将内存结点写入文件结点
//入口参数：内存结点指针
//返回值：0：正常， 1：写入内存结点失败
int Refresh_Inode(Mem_Inode_P Mem_Inode_Info)
{
    if(Mem_Inode_Info->Modified_Inode == True)
    {
        Mem_Inode_Info->Modified_Inode = False;
        if(Write_Inode(Mem_Inode_Info) != 0)
            return 1;
    }
    return 0;
}

//将磁盘结点读入内存结点
//入口参数：结点号
//返回值：内存结点指针
Mem_Inode_P Iget(int Inode_Num)
{
    Mem_Inode_P tmp;
    tmp = Mem_Inode_head;
    
    while(tmp->Next != NULL) //从内存所有结点的链表中查找是否已经有要读入的结点的节点号
    {
        if(tmp->Next->Inode_Num == Inode_Num)
        {
            if(tmp->Next->File_Open_Link == 0) //如果该结点在空闲内存结点链表上，要从空闲内存链表中移除
            {
                tmp->Next_Free = tmp->Next->Next_Free;
                tmp->Next->Next_Free = NULL;
            }
            tmp->Next->File_Open_Link++;
            return tmp->Next;
        }
        tmp = tmp->Next;
    }
    if(Mem_Inode_head->Next_Free == NULL)//没有空闲内存结点
        return NULL;
    tmp = Mem_Inode_head->Next_Free;
    Mem_Inode_head->Next_Free = tmp->Next_Free;
    tmp->Next_Free = NULL;

    Read_Inode(Inode_Num,tmp);
    
    tmp->Inode_Num = Inode_Num;
    tmp->File_Open_Link = 1;
    tmp->Modified_Inode = True;
    
    return tmp;
}

//搜索文件中的空闲结点，重新填充超级块空闲结点表
//返回值：-1，读取结点信息失败， 其他非负整数：填充到超级块空闲结点表的结点数目；
int Search_Free_Disk_Inode(void)
{
    unsigned int Offset;
    Disk_Inode tmp;
    int i = 0;
    int Search_Num;

    Search_Num = Surper_Block_Info.Free_Inode_Suffix; //超级块空闲结点表最后一项为开始搜索的节点号
    Offset = Inode_Num_to_Offset(Search_Num);
    fseek(File_System_fp,Offset,SEEK_SET);

    while(1)
    {
        if(fread(&tmp, sizeof(Disk_Inode), 1, File_System_fp) != 1)
            return -1;
        if(tmp.File_Type == 0) //文件类型为0表示结点空闲
        {
            Surper_Block_Info.Free_Inode_Table[i++] = Search_Num;
            Surper_Block_Info.Modified = True;
        }
            
        if(i >= DISK_FREE_INODE_TABLE_SIZE) //超级块空闲结点表已填充满
            return i;
        Search_Num++;
        if(Search_Num >= DISK_INODE_NUM)//超过最大节点号，从头开始
        {
            Search_Num = 0;
            fseek(File_System_fp,Surper_Block_Info.Start_Inode_Offset,SEEK_SET);
        }
        if(Search_Num == Surper_Block_Info.Free_Inode_Suffix)//搜索到开始搜索的节点号，搜索结束
            return i;
    }
}

//分配结点，即分配一个结点，然后分配一个内存结点，将结点读入内存节点
//返回值：内存结点指针
Mem_Inode_P Ialloc(void)
{
    int ret;
    int Inode_Num;
    Mem_Inode_P Mem_Inode_Info;
    char Time_Buf[30];
    while(1)
    {
        if(Surper_Block_Info.Free_Inode_Suffix == DISK_FREE_INODE_TABLE_SIZE - 1) //超级块空闲结点表的空闲结点只剩下最后一个
        {
            ret = Search_Free_Disk_Inode();
            if(ret != DISK_FREE_INODE_TABLE_SIZE) //没有填满超级块空闲结点表
                return NULL;
            Surper_Block_Info.Free_Inode_Suffix = 0;
        }
        Inode_Num = Surper_Block_Info.Free_Inode_Suffix;
        Surper_Block_Info.Free_Inode_Suffix++;

        Mem_Inode_Info = Iget(Inode_Num); //将结点读入内存结点

        if(Mem_Inode_Info->File_Type != 0) //多进程时可能会被其他的进程先得到该结点
        {
            Write_Inode(Mem_Inode_Info);
            Iput(Mem_Inode_Info);
            continue;
        }
        Surper_Block_Info.Free_Inode_Num--;
        Surper_Block_Info.Modified = True;

        return Mem_Inode_Info;
    }
}

//释放文件结点
//入口参数：节点号
void Ifree(int Inode_Num)
{
    Surper_Block_Info.Free_Inode_Num++;
    if(Surper_Block_Info.Free_Inode_Suffix == 0) //超级空闲结点表为满
    {
        if(Inode_Num < Surper_Block_Info.Free_Inode_Table[DISK_FREE_INODE_TABLE_SIZE - 1])
            Surper_Block_Info.Free_Inode_Table[DISK_FREE_INODE_TABLE_SIZE - 1] = Inode_Num;
    }
    else
    {
        Surper_Block_Info.Free_Inode_Suffix--;
        Surper_Block_Info.Free_Inode_Table[Surper_Block_Info.Free_Inode_Suffix] = Inode_Num;
    }
    Surper_Block_Info.Modified = True;
}

//释放内存结点，若该结点已经没有名字对应，即被删除时，同时需要释放文件里的结点，和分配的文件块
//入口参数：内存结点指针
void Iput(Mem_Inode_P Mem_Inode_Info)
{
    Mem_Inode_Info->File_Open_Link--;
    if(Mem_Inode_Info->File_Open_Link == 0) //该结点没有被使用了
    {
        if(Mem_Inode_Info->File_Name_Link == 0) //该结点的内容没有名字了，即被删除
        {
            Bfree_Inode(Mem_Inode_Info); //释放文件块
            Ifree(Mem_Inode_Info->Inode_Num);//释放文件结点

            Mem_Inode_Info->File_Type = 0;
            Mem_Inode_Info->Modified_Inode = True;
            Refresh_Inode(Mem_Inode_Info); //必须马上写入，因为之后的结点号改变，将不能把改变后的内存结点正确写入文件
            Mem_Inode_Info->Inode_Num = -1; //该内存结点之后只能在空闲内存结点中找到
        }
        Refresh_Inode(Mem_Inode_Info);
        Mem_Inode_Info->Next_Free = Mem_Inode_head->Next_Free;
        Mem_Inode_head->Next_Free = Mem_Inode_Info;
    }
}


//创建内存结点链表
//返回值：1:分配内存失败； 0：创建链表成功
int Creat_Mem_Inode_Table(void)
{
    int i;
    Mem_Inode_P tmp;
    Mem_Inode_head = (Mem_Inode_P)malloc(sizeof(Mem_Inode));
    if(Mem_Inode_head == NULL)
        return 1;
    tmp = Mem_Inode_head;
    for(i = 0; i < MEM_FREE_INODE_TABLE_SIZE; i++)
    {
        tmp->Next = (Mem_Inode_P)malloc(sizeof(Mem_Inode));
        if(tmp->Next == NULL)
            return 1;
        tmp->Next->Inode_Num = -1;
        tmp->Next_Free = tmp->Next;
        tmp = tmp->Next;
    }
    tmp->Next = NULL;
    tmp->Next_Free = NULL;

    return 0;
}

//更新超级块， 将超级块信息写入文件
//返回值：0：正常， 1：写入文件失败；
int Refresh_Surper_Block(void)
{
    int Write_Count;
    if(Surper_Block_Info.Modified == True)
    {
        Surper_Block_Info.Modified = False;
        fseek(File_System_fp,0,SEEK_SET);
        Write_Count = fwrite(&Surper_Block_Info,sizeof(Surper_Block),1,File_System_fp);
        if(Write_Count == 1)
            return 0;
        else
            return 1;
    }
    return 0;
}

//超级块初始化，并写入文件
//返回值：0；正常； 1：写入文件失败；
int Surper_Block_Init(void)
{
    int i;
    int Surper_Block_Size,Disk_Inode_Table_Size;
    Surper_Block_Size = sizeof(Surper_Block);
    Disk_Inode_Table_Size = sizeof(Disk_Inode) * DISK_INODE_NUM;
    Surper_Block_Info.Free_Block_Num =(FILE_SYSTEM_SIZE - Surper_Block_Size - Disk_Inode_Table_Size) / BLOCK_SIZE;
    Surper_Block_Info.Extend_Size = BLOCK_SIZE - ((Surper_Block_Size + Disk_Inode_Table_Size) % BLOCK_SIZE);
    Surper_Block_Info.Start_Block_Offset = Surper_Block_Size + Disk_Inode_Table_Size + Surper_Block_Info.Extend_Size;
    Surper_Block_Info.Free_Block_Suffix = 0;

    Surper_Block_Info.Start_Inode_Offset = Surper_Block_Size;
    Surper_Block_Info.Free_Inode_Num = DISK_INODE_NUM;
    Surper_Block_Info.Free_Inode_Suffix = 0;

    strcpy(Surper_Block_Info.ID,File_System_ID);

    for(i = 0; i < FREE_BLOCK_TABLE_SIZE; i++)
        Surper_Block_Info.Free_Block_Table[i] = i;

    for(i = 0; i < DISK_FREE_INODE_TABLE_SIZE; i++)
        Surper_Block_Info.Free_Inode_Table[i] = i;
    
    Surper_Block_Info.Modified = True;

    if(Refresh_Surper_Block() != 0)
        return 1;
    return 0;
}

//写入块链接信息
//返回值：0：正常； 1；
int Free_Block_Table_Init(void)
{
    int i,j = FREE_BLOCK_TABLE_SIZE;
    int Block_Data[FREE_BLOCK_TABLE_SIZE];

    while(1)
    {
        for(i = 0; i < FREE_BLOCK_TABLE_SIZE; i++)
        {
            Block_Data[i] = i + j;
            if(Block_Data[i] > Surper_Block_Info.Free_Block_Num)//超过最大空闲块数量
                break;
        }
        if(Write_Block_Int(j-1,Block_Data,i) != 1)
            return 1;
        printf("%.2lf%%....\n",((float)(j-1) / 1045503) * 100);
        
        if(i < FREE_BLOCK_TABLE_SIZE)//所以空闲块信息已经写入链接块
            break;
        j += FREE_BLOCK_TABLE_SIZE;//1045504
    }
    return 0;
}

//初始化磁盘结点表，将所有的磁盘结点设为空闲，即文件类型为0，并写入文件
//返回值：0：正常； 1；写入文件失败
int Free_Inode_Table_Init(void)
{
    int i,Write_Count;
    Disk_Inode Disk_INode_Info;
    Disk_INode_Info.File_Type = 0;
    fseek(File_System_fp,Surper_Block_Info.Start_Inode_Offset,SEEK_SET);
    for(i = 0; i < DISK_INODE_NUM; i++)
    {
        Write_Count = fwrite(&Disk_INode_Info,sizeof(Disk_Inode),1,File_System_fp);
        if(Write_Count != 1)
            return 1;
    }
    return 0;
}

//创建文件系统
//返回值：0：正常； 1：超级快初始化失败； 2；块链接信息初始化失败； 3：磁盘结点初始化失败； 4；创建根目录失败；
int Creat_File_System(void)
{
    if(Surper_Block_Init() != 0)
        return 1;
    if(Free_Block_Table_Init() != 0)
        return 2;
    if(Free_Inode_Table_Init() != 0)
        return 3;
    if(Creat_Root_Dirent() != 0)
        return 4;
    return 0;
}

int Read_File_System(void)
{
    int ret;
    if(((ret = fread(&Surper_Block_Info, sizeof(Surper_Block), 1, File_System_fp)) == 1) && strcmp(Surper_Block_Info.ID, File_System_ID) == 0) 
        return 0;
    return 1;
}

int Print_File_System_Info(void)
{
    long int My_File_System_Size = FILE_SYSTEM_SIZE;
    long int Used_size, Available_Size;

    Available_Size = (long)Surper_Block_Info.Free_Block_Num * BLOCK_SIZE;
    Used_size = My_File_System_Size - Available_Size;

    printf("\n/*******************%s***********************/\n", Surper_Block_Info.ID);
    printf("File System Capacity: %.2f GB\n",(float) My_File_System_Size / 1024 / 1024 / 1024);

    if(Available_Size < 1024 * 1024)
        printf("Available Capacity: %.2f KB\n", (float)Available_Size / 1024);
    else if(Available_Size < 1024 * 1024 * 1024)
        printf("Available Capacity: %.2f MB\n", (float)Available_Size / 1024 / 1024);
    else
        printf("Available Capacity: %.2f GB\n", (float)Available_Size / 1024 / 1024 / 1024);

    if(Used_size < 1024)
        printf("Used Capacity: %ld KB\n", Used_size);
    else if(Used_size < 1024 * 1024)
        printf("Used Capacity: %.2f KB\n", (float)Used_size / 1024);
    else if(Used_size < 1024 * 1024 * 1024)
        printf("Used Capacity: %.2f MB\n", (float)Used_size / 1024 / 1024);
    else
        printf("Used Capacity: %.2f GB\n", (float)Used_size / 1024 / 1024 / 1024);

    printf("/********************************************************/\n");
}

//打开文件系统，文件系统存在就读取，不存在则创建
//入口参数：Path：文件系统路径
//返回值：-1：文件创建失败； 
int Open_File_System(char *Path)
{
    int ret;
    Creat_Mem_Inode_Table();
    if((File_System_fp = fopen(Path,"r+")) == NULL)
    {
        printf("%s File_System not exit !\nCreat a File_System...\n", Path);
        if((File_System_fp = fopen(Path,"w+")) == NULL)
        {
            printf("File_System Creat Fail !");
            return -1;
        }
        if((ret = Creat_File_System()) != 0)
        {
            printf("File_System Init Fail !");
            return 0;
        }
        else
            printf("File_System Init Success !\n");
    }
    else
    {
        if((ret = Read_File_System()) == 0)
            printf("Read_File_System Success !\n");
        else
        {
            printf("Read_File_System Fail !\n");
            return 0;
        }
            
    }
    Current_Dirent_Inode_Num = Surper_Block_Info.Dirent_Root_Inode_Num;
    Print_File_System_Info();
    
    return ret;
}

int Close_File_System(void)
{
    if(Refresh_Surper_Block() != 0)
        return 1;;
    fclose(File_System_fp);

    return 0;
}

int Init_File_System_Part(void)
{
    if(Surper_Block_Init() != 0)
        return 1;
    if(Free_Inode_Table_Init() != 0)
        return 3;
    if(Creat_Root_Dirent() != 0)
        return 4;
    return 0;
}

void Get_Time(char *Time_Buf)
{
    time_t Time_Now;
    struct tm *tmp;

    time(&Time_Now);
    tmp = localtime(&Time_Now);
    strftime(Time_Buf,32,"%R, %a %b %d, %Y",tmp);
}

int Write_Dirent_Item(Mem_Inode_P Mem_Inode_Num, Dirent_Item Dirent_Item_S)
{
    int Offset,Useful_Size;
    int Dirent_Item_Size = sizeof(Dirent_Item);
    int Dirent_Item_Address;

    Dirent_Item_Address = Dirent_Item_S.Num * Dirent_Item_Size;
    File_Bmap(Dirent_Item_Address,Mem_Inode_Num,&Offset,&Useful_Size,True);

    if(Useful_Size < Dirent_Item_Size)
        return 2;
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fwrite(&Dirent_Item_S, sizeof(Dirent_Item), 1, File_System_fp) != 1)
        return 1;
    if(Dirent_Item_Address >= Mem_Inode_Num->File_Size)
    {
        Mem_Inode_Num->File_Size += Dirent_Item_Size;
        Mem_Inode_Num->Modified_File = True;
    }
    
    return 0;
}


int Creat_Root_Dirent(void)
{
    char Time_Buf[30];
    int i;
    Mem_Inode_P Mem_Inode_Num;
    Dirent_Item Dirent_Item_S[2];

    Mem_Inode_Num = Ialloc();
    Surper_Block_Info.Dirent_Root_Inode_Num = Mem_Inode_Num->Inode_Num;
    Surper_Block_Info.Modified = True;
    Get_Time(Time_Buf);
    strcpy(Mem_Inode_Num->File_Creat_Time,Time_Buf);
    strcpy(Mem_Inode_Num->File_Access_Time,Time_Buf);
    strcpy(Mem_Inode_Num->File_Modifie_Time,Time_Buf);
    strcpy(Mem_Inode_Num->Inode_Modifie_Time,Time_Buf);
    Mem_Inode_Num->Modified_Inode = True;
    Mem_Inode_Num->File_Type = TYPE_DIR;
    Mem_Inode_Num->File_Name_Link = 1;
    Mem_Inode_Num->File_Size = 0;

    for(i = 0; i < 13; i++)
        Mem_Inode_Num->File_Disk_Address[i] = INDEX_BLOCK_FREE;
    
    strcpy(Dirent_Item_S[0].Name,".");
    Dirent_Item_S[0].Inode_Num = Mem_Inode_Num->Inode_Num;
    Dirent_Item_S[0].Num = 0;
    strcpy(Dirent_Item_S[1].Name,"..");
    Dirent_Item_S[1].Inode_Num = Mem_Inode_Num->Inode_Num;
    Dirent_Item_S[1].Num = 1;

    Mem_Inode_Num->Modified_File = True;

    if(Write_Dirent_Item(Mem_Inode_Num, Dirent_Item_S[0]) != 0)
        return 1;
    if(Write_Dirent_Item(Mem_Inode_Num, Dirent_Item_S[1]) != 0)
        return 1;

    if(Refresh_Inode(Mem_Inode_Num) != 0)
        return 1;
    if(Refresh_Surper_Block() != 0)
        return 1;
    Iput(Mem_Inode_Num);
    return 0;
}

int Get_Path_Name(char *Full_Path, char *Path_Name, int Order)
{
    int tmp = 0,i,j = 0;
    Bool End = False;
    while(1)
    {
        i = 0;
        while(1)
        {
            Path_Name[i] = Full_Path[j];
            if(Path_Name[i] == '/')
            {
                if(j == 0)
                {
                    i += 1;
                    Path_Name[i] = '\0';
                    if(Full_Path[j+1] == '\0')
                        End = True;
                }
                else
                    Path_Name[i] = '\0';
                j++;
                break;
            }
            if(Path_Name[i] == '\0')
            {
                End = True;
                break;
            }
            i++;
            j++;
        }
        if(End == True)
        {
            if(tmp >= Order)
                return 0;
            else
                return 1;
        }
        if(tmp >= Order)
            return 0;
        tmp++;
    }
}

int Get_Path_Len(char *Path_Name)
{
    int i = 0;
    int Len = 0;
    while(1)
    {
        if(Path_Name[i] == '/' && i !=0)
            Len++;
        else if(Path_Name[i] == '/' && Path_Name[i + 1] == '\0')
            return Len;
        if(Path_Name[i] == '\0')
            break;
        i++;
    }
    return Len+1;
}

int Get_Path_Inode(char *Path_Name, Mem_Inode_P *Opendir_Inode_Info)
{
    int Path_Len;
    int i,order;
    int ret;
    char Dirent_Name[NAME_MAX_LENGTH];
    Dirent_Item Dirent_Item_S;

    Path_Len = Get_Path_Len(Path_Name);

    if(Path_Name[0] == '/')
    {
        *Opendir_Inode_Info = Iget(Surper_Block_Info.Dirent_Root_Inode_Num);
        
        order = 1;
    }
    else
    {
        *Opendir_Inode_Info = Iget(Current_Dirent_Inode_Num);
        order = 0;
    }
    Path_Len--;
    while(1)
    {
        if(Get_Path_Name(Path_Name,Dirent_Name,order) != 0)
            break;
        i = 2;
        while((ret = Readir_S(*Opendir_Inode_Info, &Dirent_Item_S,i)) == 0)
        {
            if(strcmp(Dirent_Name,Dirent_Item_S.Name) == 0)
            {
                Iput(*Opendir_Inode_Info);
                *Opendir_Inode_Info = Iget(Dirent_Item_S.Inode_Num);
                break;
            }
            i++;
        }
        if(ret != 0)
        {
            Iput(*Opendir_Inode_Info);
            return 1; //未找到
        }
            
        if((*Opendir_Inode_Info)->File_Type != TYPE_DIR)
        {
            if(Path_Len > 0)
            {
                Iput(*Opendir_Inode_Info);
                return 2;//路径中除最后分量外还存在非目录文件
            }
        }
        if(Path_Len == 0)
            return 0;//正常
        Path_Len--;
        order++;
    }
    return 0;
}

int Opendir_S(char *Path_Name, Mem_Inode_P *mem_Inode_Info)
{
    int i = 0,ret;

    if((ret = Get_Path_Inode(Path_Name,mem_Inode_Info)) != 0)
    {
        if(ret == 1)
            printf("Path Name Not Exit!\n");
        else if(ret == 2)
            printf("Path Name is Wrong!\n");
        return 1;
    }
    if((*mem_Inode_Info)->File_Type != TYPE_DIR)
    {
        printf("Path Name Is Not Dirent\n");
        return 2;
    }
    return 0;
}

int Readir_S(Mem_Inode_P Mem_Inode_Info, Dirent_Item_P Dirent_Item_P, int Dirent_Item_Num)
{
    int Dirent_Item_Offset = Dirent_Item_Num * sizeof(Dirent_Item);
    int Offset,Useful_Size;
    if(Dirent_Item_Offset >= Mem_Inode_Info->File_Size)
        return 1;
    File_Bmap(Dirent_Item_Offset,Mem_Inode_Info,&Offset,&Useful_Size,False);
    fseek(File_System_fp,Offset,SEEK_SET);
    if(fread(Dirent_Item_P, sizeof(Dirent_Item), 1, File_System_fp) != 1)
        return 2;
    return 0;
}

int Delete_Dirent_Item(Mem_Inode_P Dir_Inode_Info, Dirent_Item Dirent_Item_S)
{
    Dirent_Item Dirent_Item_tmp;
    int i = Dirent_Item_S.Num + 1;
    
    while(Readir_S(Dir_Inode_Info, &Dirent_Item_tmp,i) == 0)
    {
        Dirent_Item_tmp.Num--;
        if(Write_Dirent_Item(Dir_Inode_Info, Dirent_Item_tmp) != 0)
            return 1;
        i++;
    }
    Dir_Inode_Info->File_Size = Dir_Inode_Info->File_Size - sizeof(Dirent_Item);
    Dir_Inode_Info->Modified_Inode = True;
    return 0;
}

void Remove_File_Name(char *Path_Name, char *File_Name)
{
    int Path_Len,i;
    Path_Len = Get_Path_Len(Path_Name);
    Get_Path_Name(Path_Name,File_Name,Path_Len);

    for(i = strlen(Path_Name) - 1; i >= 0; i--)
    {
        if(Path_Name[i] == '/')
        {
            if(i != 0)
                Path_Name[i] = '\0';
            break;
        }
        Path_Name[i] = '\0';
    }
}

int Creat_S(char *Path_Name)
{
    char Time_Buf[30];
    int i = 2;
    char File_Name[NAME_MAX_LENGTH];
    char Dirent_Name[NAME_MAX_LENGTH];
    Mem_Inode_P Mem_Inode_Num;
    Mem_Inode_P Dir_Inode_Info;
    Dirent_Item Dirent_Item_S;

    strcpy(Dirent_Name,Path_Name);
    Remove_File_Name(Dirent_Name,File_Name);

    if(Opendir_S(Dirent_Name, &Dir_Inode_Info) != 0)
        return 1; //打开目录失败
    
    while(Readir_S(Dir_Inode_Info, &Dirent_Item_S,i) == 0)
    {
        if(strcmp(Dirent_Item_S.Name, File_Name) == 0)
            return 2;
        i++;
    }

    Mem_Inode_Num = Ialloc();
    Get_Time(Time_Buf);
    strcpy(Mem_Inode_Num->File_Creat_Time,Time_Buf);
    strcpy(Mem_Inode_Num->File_Access_Time,Time_Buf);
    strcpy(Mem_Inode_Num->File_Modifie_Time,Time_Buf);
    strcpy(Mem_Inode_Num->Inode_Modifie_Time,Time_Buf);
    Mem_Inode_Num->Modified_Inode = True;
    Mem_Inode_Num->File_Type = TYPE_REG;
    Mem_Inode_Num->File_Name_Link = 1;
    Mem_Inode_Num->File_Size = 0;

    for(i = 0; i < 13; i++)
        Mem_Inode_Num->File_Disk_Address[i] = INDEX_BLOCK_FREE;
    
    Dirent_Item_S.Inode_Num = Mem_Inode_Num->Inode_Num;
    Dirent_Item_S.Num = Dir_Inode_Info->File_Size / sizeof(Dirent_Item);
    strcpy(Dirent_Item_S.Name,File_Name);

    if(Write_Dirent_Item(Dir_Inode_Info, Dirent_Item_S) != 0)
        return 3;//写入目录失败

    if(Refresh_Inode(Dir_Inode_Info) != 0)
        return 4;//更新节点失败
    if(Refresh_Inode(Mem_Inode_Num) != 0)
        return 4;//更新节点失败
    if(Refresh_Surper_Block() != 0)
        return 4;//更新超级快失败
    Iput(Mem_Inode_Num);
    Iput(Dir_Inode_Info);
    
    return 0;
}

int Unlink_S(char *Path_Name)
{
    int i = 2;
    char File_Name[NAME_MAX_LENGTH];
    char Dirent_Name[NAME_MAX_LENGTH];
    Mem_Inode_P File_Inode_Info;
    Mem_Inode_P Dir_Inode_Info;
    Dirent_Item Dirent_Item_S;

    strcpy(Dirent_Name,Path_Name);
    Remove_File_Name(Dirent_Name,File_Name);

    if(Opendir_S(Dirent_Name, &Dir_Inode_Info) != 0)
        return 1; //打开目录失败

    while(Readir_S(Dir_Inode_Info, &Dirent_Item_S,i) == 0)
    {
        if(strcmp(Dirent_Item_S.Name, File_Name) == 0)
        {
            File_Inode_Info = Iget(Dirent_Item_S.Inode_Num);
            File_Inode_Info->File_Name_Link--;
            if(Delete_Dirent_Item(Dir_Inode_Info,Dirent_Item_S)!= 0)
                return 1;//删除目录项失败
            Iput(Dir_Inode_Info);
            Iput(File_Inode_Info);
        }
        i++;
    }
    return 0;
}

int ls(char cmd, char *Path_Name)
{
    int i = 0;
    Mem_Inode_P Mem_Inode_Info;
    Dirent_Item Dirent_Item_S;
    Mem_Inode_P Mem_Inode_Num;
    char Dirent_Name[NAME_MAX_LENGTH];
    char Str_Space = ' ';
    Bool First_Sta = True;

    if(Opendir_S(Path_Name, &Mem_Inode_Info) != 0)
        return 2;
    if(strcmp(Path_Name, "/") == 0)
        strcpy(Dirent_Name,"Root\0");
    else
        strcpy(Dirent_Name,Path_Name);
    printf("Current Dirent: %s\n",Dirent_Name);
    while(Readir_S(Mem_Inode_Info, &Dirent_Item_S,i) == 0)
    {
        switch(cmd)
        {
            case 'd':
                printf("%s ",Dirent_Item_S.Name);
            break;
            case 'l':
                if(First_Sta)
                {
                    First_Sta = 0;
                    printf("Name%-16cType%-6cLink%-6cSize%-6cModified Time%c\n",Str_Space,Str_Space,Str_Space,Str_Space,Str_Space);
                }
                Mem_Inode_Num = Iget(Dirent_Item_S.Inode_Num);
                printf("%-20s%-10d%-10d%-10u%s\n",Dirent_Item_S.Name, Mem_Inode_Num->File_Type, Mem_Inode_Num->File_Name_Link, Mem_Inode_Num->File_Size,Mem_Inode_Num->Inode_Modifie_Time);
                Iput(Mem_Inode_Num);
            break;
            case 'u':
                if(First_Sta)
                {
                    First_Sta = 0;
                    printf("Name%-16cInode%-5cType%-6cLink%-6cSize%-6cModified Time%c\n",Str_Space,Str_Space,Str_Space,Str_Space,Str_Space,Str_Space);
                }
                Mem_Inode_Num = Iget(Dirent_Item_S.Inode_Num);
                printf("%-20s%-10d%-10d%-10d%-10u%s\n",Dirent_Item_S.Name,Mem_Inode_Num->Inode_Num,Mem_Inode_Num->File_Type, Mem_Inode_Num->File_Name_Link, Mem_Inode_Num->File_Size,Mem_Inode_Num->Inode_Modifie_Time);
                Iput(Mem_Inode_Num);
            break;
            default:
                printf("Wrong Cmd\n");
                return 1;
        }
        i++;
    }
    Iput(Mem_Inode_Info);
    return 0;
}