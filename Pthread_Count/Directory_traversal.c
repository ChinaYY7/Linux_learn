#include "Directory_traversal.h"
#include "apue.h"
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>

#include "Pthread_count.h"

extern struct job Thread_job[Job_num];
extern struct queue Job_queue;


static char *Fullpath;
int Path_len = 1024;

void Allocate_Pathname_Mem(char *Pathname)
{
    if(Path_len < strlen(Pathname))  //如果路径长度大小比设定的最大路径长度还长，就多分配些内存
        Path_len *=2;
    Fullpath = malloc(sizeof(char) * Path_len);                       
    strcpy(Fullpath, Pathname);  //路径名保存到分配的内存中
    Fullpath[Path_len-1] = '\0'; //字符串结尾
}                                       
void Free_Pathname_Mem(void)
{
    free(Fullpath);
}

int Descending_Traversal_Directory(void)
{
    struct stat statbuf;  //保存文件信息
    struct dirent *dirp;  //目录内容
    DIR *dp;              //目录有关信息
    int ret,n;            //返回值，当前路径长度

    if (lstat(Fullpath, &statbuf) < 0)                  //读取文件信息
        return(Process_File_Type(Fullpath, &statbuf, FTW_NS));       //文件信息读取失败

    if (S_ISDIR(statbuf.st_mode) == 0)                  //是不是目录        
        return(Process_File_Type(Fullpath, &statbuf, FTW_F));        //不是目录

    if ((ret = Process_File_Type(Fullpath, &statbuf, FTW_D)) != 0)   //对目录文件计数
        return(ret);                                    //func程序出错
    
    n = strlen(Fullpath);                               //路径长度

    if (n + _PC_NAME_MAX + 2 > Path_len)                //路径长度动态变化，预估之后的路径长度有没有超过之前分配的内存大小的可能
    {
        Path_len *=2;
        if((Fullpath = realloc(Fullpath ,Path_len)) == NULL)
            perror("realloc failed\n");
    }
    Fullpath[n++] = '/';                                //路径名后添加符号'/',划分路径分量的
    Fullpath[n] = 0;                                    //路径以0为结尾
    if((dp = opendir(Fullpath)) == NULL)                //打开目录
        return(Process_File_Type(Fullpath, &statbuf, FTW_DNR));      //打开目录失败
    while ((dirp = readdir(dp)) != NULL)                //读取目录内容
    {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)//跳过.和..  不需要遍历
            continue;
        strcpy(&Fullpath[n], dirp->d_name);             //将读到的目录内容，即该目录下文件的文件名，添加到路径中
        if ((ret = Descending_Traversal_Directory()) != 0)                  //递归调用本身
            break;                                      //出错
    }
    Fullpath[n-1] = 0;                                  //去掉'/'
    if (closedir(dp) < 0)                               //关闭目录
        fprintf(stderr, "can't close directory %s\n", Fullpath);
    return ret;
}

void Print_Find_Result(char *Pathname)
{
    Pathname[strlen(Pathname)] = '\0';
    printf("%s\n", Pathname);
}

int Process_File_Type(const char *Pathname, const struct stat *statptr, int type)
{  
    
    switch (type)
    {
        case FTW_F:
            if((statptr->st_mode & S_IFMT) == S_IFREG)
            {
                Assign_Job(Pathname);
                //Print_Find_Result(Fullpath);
            }
            break;
        case FTW_D:
            break;
        case FTW_DNR:
            fprintf(stderr, "can't read directory %s\n", Pathname);
            break;
        case FTW_NS:
            fprintf(stderr, "stat error for %s\n", Pathname);
            break;
        default:
            fprintf(stderr, "unkown type %d for Pathname %s\n", type, Pathname);
    }
    return 0;
}

void Find_Reg_File(char *Pathname)
{
    int ret;
    Allocate_Pathname_Mem(Pathname);
    ret = Descending_Traversal_Directory();
    Free_Pathname_Mem();
    if(ret != 0)
        printf("Find Regular File Fail\n");
}
//-------------------------------------------------------------

void Assign_Job(const char *Pathname)
{
    int Free_job_suffix;
    static int Free_job_num = 0;
    int n;

    do
        Free_job_suffix = Find_Free_job(Thread_job);
    while(Free_job_suffix == Job_num);

    if(Thread_job[Free_job_suffix].Path_men_sta == False)
    {
        Thread_job[Free_job_suffix].File_Path = malloc(sizeof(char) * Path_len);
        Thread_job[Free_job_suffix].Path_men_sta = True;
        Thread_job[Free_job_suffix].Path_Len = Path_len;
    }

    n = strlen(Pathname);
    
    if (n + _PC_NAME_MAX + 2 > Thread_job[Free_job_suffix].Path_Len)                //路径长度动态变化，预估之后的路径长度有没有超过之前分配的内存大小的可能
    {
        Thread_job[Free_job_suffix].Path_Len *= 2;
        if((Thread_job[Free_job_suffix].File_Path = realloc(Thread_job[Free_job_suffix].File_Path ,Path_len)) == NULL)
            perror("realloc failed\n");
    }

    strcpy(Thread_job[Free_job_suffix].File_Path,Pathname);
    Thread_job[Free_job_suffix].j_id = Free_job_num % Thread_num;
    Thread_job[Free_job_suffix].END = False;
    Thread_job[Free_job_suffix].Free = False;
    job_append(&Job_queue,&Thread_job[Free_job_suffix]);
    Free_job_num++;
}


