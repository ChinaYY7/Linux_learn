//time ./Pro_Cus1.4.out /mnt/f/Linux_code/tmp/11.mp4 /mnt/f/Linux_code/tmp/22.mp4
//time ./Pro_Cus1.6.out /mnt/f/Linux_code/tmp/tmp.c /mnt/f/Linux_code/tmp/tmp1.c
//md5sum ../tmp/tmp1.c ../tmp/tmp.c
#include "apue.h"
#include <sys/time.h> 
#include <sys/wait.h>
#define FIFO_path "/home/mr_yy/Program/FIFO/fifo"
#define Pro_FIFO_path "/home/mr_yy/Program/FIFO/fifo1"
#define Cus_FIFO_path "/home/mr_yy/Program/FIFO/fifo2"
#define BUFFER_SIZE 4096 //FIFO大小4096，流缓冲大小8192  //需设为管道大小的整数倍
//#define BUFFER_SIZE PIPE_BUF //4096
#define Pro_child_Num 5
#define Cus_child_Num 5


//出错处理
void Error_Exit(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

//创建pipe
void Tell_Wait_pipe(int *fd)
{
    if (pipe(fd) < 0)
        Error_Exit("Creat pipe error\n");
    if (write(fd[1],"c",1) != 1)
        Error_Exit("Write pipe error\n");
}

//读pipe
void Wait_pipe(int *fd)
{
    char temp;
    if (read(fd[0], &temp, 1) != 1)
        Error_Exit("Read pipe error\n");
}

//写pipe
void Tell_pipe(int *fd)
{
    if (write(fd[1],"c",1) != 1)
        Error_Exit("Write pipe error!\n\n");
}
//主程序
int main(int argc, char const *argv[])
{
    pid_t pid[3],MD5_pid;  
    int res;
    int End = 2;
    struct timeval start,finish;
    int Pro_child_FIFO_fd;
    char Pro_date[2] = {0,0};

    gettimeofday(&start,NULL);//获取程序开始时间

    //检查参数
    if (argc != 3) 
    {
        fprintf(stderr, "Wrong number of parameters: %d != 3\n", argc);  
        exit(EXIT_FAILURE);
    }

    //管道是否存在以及创建有名管道FIFO,子进程间通讯
    if((mkfifo(Pro_FIFO_path, FILE_MODE)) < 0 && (errno != EEXIST))
        Error_Exit("Can not creat FIFO\n");

    if ((Pro_child_FIFO_fd = open(Pro_FIFO_path, O_RDWR)) == -1)
        Error_Exit("Open fifo error\n");
    if(write(Pro_child_FIFO_fd, Pro_date, 1) == -1)
        Error_Exit("Write FIFO error\n");

    //管道是否存在以及创建有名管道FIFO
    if((mkfifo(FIFO_path, FILE_MODE)) < 0 && (errno != EEXIST))
        Error_Exit("Can not creat FIFO\n");
    
    //创建生产者进程
    if((pid[1] = fork()) < 0)
        Error_Exit("fork1 error\n");
        
    //生产者进程
    else if(pid[1] == 0)
    {
        int i;
        int Source_fd;
        FILE *Pro_fp;
        int Pro_FIFO_fd;
        pid_t Pro_pid[Pro_child_Num + 1];
        int Pro_End = Pro_child_Num;
        static int Pro_Pipe_fd[2];
        char Pro_chr;

        //创建无名管道
        Tell_Wait_pipe(Pro_Pipe_fd);

        //读打开源文件
        printf("--Producer<%d> created successfully\n--Open source file <%s>\n", getpid(), argv[1]);
        if((Pro_fp = fopen(argv[1],"r")) == NULL)
            Error_Exit("Open source file error\n");
        
        //以写方式打开FIFO
        printf("--Producer<%d> opening FIFO O_WRONLY\n--Waiting Customer.....\n\n", getpid());
        if ((Pro_FIFO_fd = open(FIFO_path, O_WRONLY)) == -1)
            Error_Exit("Open fifo error\n");
        
        //创建生产者子进程将源文件数据写入管道
        printf("--Producer<%d> read from %s and start writing in fifo<%d>\n", getpid(), argv[1], Pro_FIFO_fd); 
        for(i = 1; i < Pro_child_Num + 1; i++)
        {
            if((Pro_pid[i] = fork()) < 0)
                Error_Exit("Pro_fork error\n");
            
            //生产者子进程
            else if(Pro_pid[i] == 0)
            {
                int Pro_count;
                long int Pro_bytes;
                char Pro_buffer[BUFFER_SIZE + 1];
                printf("----Producer_child[%d]<%d> created successfully\n", i, getpid());
                Pro_bytes = 0;
                while(1)
                {
                    while(read(Pro_child_FIFO_fd, Pro_date, 1) == 0)
                    {
                        if(errno != EAGAIN)
                            break;
                    }
                    Pro_date[0]++;
                    if(write(Pro_child_FIFO_fd, Pro_date, 1) == -1)
                        Error_Exit("Write FIFO error\n");
                    //互斥
                    Wait_pipe(Pro_Pipe_fd);
                    Pro_count = fread(Pro_buffer,sizeof(char),BUFFER_SIZE,Pro_fp);
                    if(write(Pro_FIFO_fd, Pro_buffer, Pro_count) == -1)
                        Error_Exit("Write FIFO error\n");
                    Tell_pipe(Pro_Pipe_fd);
                    //互斥
                    Pro_bytes += Pro_count;
                
                    if(Pro_count != BUFFER_SIZE)
                    {
                        if (ferror(Pro_fp))
                            Error_Exit("fread error\n");
                        else
                            break;  
                    }
                    printf("Pro_date:%d\n",Pro_date[0]);
                }
                
                printf("----Producer_child[%d]<%d> have written %ld bytes \n", i, getpid(), Pro_bytes);
                exit(EXIT_SUCCESS);
            }
        }

        //等待所有生产者子进程结束
        while(Pro_End)
        {
            Pro_pid[0] = wait(NULL);
            Pro_End--;
            printf("--Producer_child[%d] end\n", Pro_pid[0]);
        }
        fclose(Pro_fp);
        close(Pro_FIFO_fd);
        exit(EXIT_SUCCESS);
    }


    //创建消费者进程
    if((pid[2] = fork()) < 0)
    {
        perror("fork2 error\n");
        exit(EXIT_FAILURE);
    }
    //消费者进程
    else if(pid[2] == 0)
    {
        int j;
        int target_fd;
        FILE *Cus_fp;
        int Cus_FIFO_fd;
        pid_t Cus_pid[Cus_child_Num + 1];
        int Cus_End = Cus_child_Num;
        static int Cus_Pipe_fd[2];
        char Cus_chr;
        long int cus_wcount;

        //创建无名管道
        Tell_Wait_pipe(Cus_Pipe_fd);
        
        //创建目标文件并写打开
        printf("--Customer %d created successfully\n--Creat target file <%s>\n", getpid(), argv[2]);
        if((Cus_fp = fopen(argv[2],"w")) == NULL)
            Error_Exit("Creat target file error\n");
        
        //以读方式打开FIFO
        printf("--Customer<%d> opening FIFO O_RDONLY\n--Waiting Producer.....\n\n", getpid());
        Cus_FIFO_fd = open(FIFO_path, O_RDONLY);
        if (Cus_FIFO_fd == -1)
            Error_Exit("Open fifo error\n");
        
        //创建消费者子进程将管道数据写入目标文件
        printf("--Customer<%d> read from fifo<%d> and start writing in file<%s>\n", getpid(), Cus_FIFO_fd, argv[2]); 
        for(j = 1; j < Cus_child_Num + 1; j++)
        {
            if((Cus_pid[j] = fork()) < 0)
                Error_Exit("Cus_fork error\n");
            //消费者子进程
            else if(Cus_pid[j]==0)
            {
                int Cus_count;
                long int Cus_bytes;
                char Cus_buffer[BUFFER_SIZE + 1];
                printf("----Customer_child[%d]<%d> created successfully\n", j, getpid());
                Cus_bytes = 0;
                while(1)
                {
                    //互斥
                    Wait_pipe(Cus_Pipe_fd);
                    if((Cus_count = read(Cus_FIFO_fd, Cus_buffer, BUFFER_SIZE)) == -1)
                        Error_Exit("Read FIFO error\n");
                    cus_wcount = fwrite(Cus_buffer,sizeof(char),Cus_count,Cus_fp);
                    printf("-----------------------------------------cus_wcount<%d> : %ld\n",getpid(),cus_wcount);
                    //必须冲洗缓冲区，不然多进程读会出现在缓冲区数据的次序不一致
                    if(fflush(Cus_fp) != 0)
                        Error_Exit("fflush error\n");

                    Tell_pipe(Cus_Pipe_fd);
                    //互斥
                    Cus_bytes += Cus_count;
                    if(Cus_count == 0)
                        break;   
                }
                printf("----Customer_child[%d]<%d> have read %ld bytes \n", j, getpid(), Cus_bytes);
                //Read_bytes += Cus_bytes;
                exit(EXIT_SUCCESS);
            }
        }
        
        //等待所有消费者子进程结束
        while(Cus_End)
        {
            Cus_pid[0] = wait(NULL);
            Cus_End--;
            printf("--Customer_child[%d] end\n", Cus_pid[0]);
        }
        fclose(Cus_fp);
        close(Cus_FIFO_fd);
        exit(EXIT_SUCCESS);
    }
    
    //等待生产者消费者完成
    while(End)
    {
        pid[0] = wait(NULL);
        End--;
        printf("Process[%d] end\n", pid[0]);
    }

    //删除FIFO
    unlink(FIFO_path);

    gettimeofday(&finish,NULL);
    printf("\nCopy complated !\ntime = %.3f s\n",(double)((finish.tv_sec-start.tv_sec) * 1000000 + (finish.tv_usec-start.tv_usec)) / 1000000);

    //printf("Producer write %ld KB\n ", Write_bytes);
    //printf("Customer read %ld KB\n", Read_bytes);

    //md5计算
    if((MD5_pid = fork()) < 0)
        Error_Exit("fork3 error\n");
    else if(MD5_pid == 0)
    {
        printf("\nCompute MD5: \n");
        if(execl("/usr/bin/md5sum","md5sum",argv[1],argv[2],(char *)0) <  0)
            Error_Exit("execl md5sum error\n");
        exit(EXIT_SUCCESS);
    }
    waitpid(MD5_pid,NULL,0);
    exit(EXIT_SUCCESS); 
}

