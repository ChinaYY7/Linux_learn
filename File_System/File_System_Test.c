#include "apue.h"
#include "File_System.h"
#define Path "/mnt/f/Linux_code/File_System/File_System.Fsys"

int main(void)
{
    int error;
    int sta = 0;
    int i;
    char Write_Buf[4096] = {"Test"};
    char Read_Buf[4096];
    char Inode_Name[100];
    Mem_Inode_P Mem_Inode_Info;
    
    Open_File_System(Path);

    for(i =0 ; i < 4096; i++)
        Write_Buf[i] = i % 10 + '0';
    //Init_File_System_Part();
    if(sta == 1)
    {
        if((error = Creat_S("test6.c")) != 0)
            printf("Creat File Fail!\nError Code:%d\n",error);
    }
    if(sta == 2)
    {
        Unlink_S("/test2.c");
    }
    //if(Mkdir_S("/Document1/Document2") != 0)
        //printf("File already exit!\n");
    //Open_S("/Document1/Document2/test.c",&Mem_Inode_Info);
    //Write_S(Mem_Inode_Info,Write_Buf,4096);
    //Read_S(Mem_Inode_Info,Read_Buf,10);
    //Close_s(Mem_Inode_Info);
    //Read_Buf[10] = '\0';
    //printf("\nRead_Buf = %s\n",Read_Buf);
    //Inode_To_Name(3,Inode_Name);
    //printf("Inode_Name : %s\n",Inode_Name);
    //Cd_S(".");
    //if(sta == 3)
        ls('u',NULL);

    //File_Stat("/test1.c");
    Close_File_System();
    //remove(Path);
}