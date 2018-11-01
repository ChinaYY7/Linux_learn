#include "apue.h"
#include "File_System.h"
#define Path "/mnt/f/Linux_code/File_System/File_System.Fsys"

int main(void)
{
    int error;
    int sta = 1;
    Open_File_System(Path);
    //Init_File_System_Part();
    if(sta == 1)
    {
        if((error = Creat_S("/test.c")) != 0)
            printf("Creat File Fail!\nError Code:%d\n",error);
    }
    if(sta == 2)
    {
        Unlink_S("/test.c");
    }
    ls('u',"/");
    Close_File_System();
    //remove(Path);
}