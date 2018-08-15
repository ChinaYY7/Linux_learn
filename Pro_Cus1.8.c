//time ./Pro_Cus1.7.out /mnt/f/Linux_code/tmp/11.mp4 /mnt/f/Linux_code/tmp/22.mp4
//time ./Pro_Cus1.7.out /mnt/f/Linux_code/tmp/tmp.c /mnt/f/Linux_code/tmp/tmp1.c
//md5sum ../tmp/tmp1.c ../tmp/tmp.c
//gdb Pro_Cus1.7.out
//set args /mnt/f/Linux_code/tmp/tmp.c /mnt/f/Linux_code/tmp/tmp1.c
//display /x {str[0],str[1],str[2],str[3]}
//使用文件偏移量
#include "apue.h"
#include <sys/time.h> 
#include <sys/wait.h>
#include <sys/shm.h>
#define FIFO_path "/home/mr_yy/Program/FIFO/fifo"
#define BUFFER_SIZE 8192 //FIFO大小4096，流缓冲大小8192  //需设为管道大小的整数倍
#define SHM_SIZE 4096
#define SHM_MODE 0600 //USR read and write
#define Pro_child_Num 5
#define Cus_child_Num 5


//出错处理
void Error_Exit(const char *str)
{
    perror(str);
    exit(EXIT_FAILURE);
}

//创建pipe
void Tell_Wait_pipe(int *fd, int num)
{
    if (pipe(fd) < 0)
        Error_Exit("Creat pipe error\n");
    while(num)
    {
        if (write(fd[1],"c",1) != 1)
            Error_Exit("Write pipe error\n");
        num--;
    }
}

//读pipe
void Wait_pipe(int *fd, int num)
{
    char temp;

    while(num)
    {
        if (read(fd[0], &temp, 1) != 1)
            Error_Exit("Read pipe error\n");
        num--;
    }
}

//写pipe
void Tell_pipe(int *fd, int num)
{
    while(num)
    {
        if (write(fd[1],"c",1) != 1)
            Error_Exit("Write pipe error\n");
        num--;
    }
}

//关闭pipe
void Close_Tell_Wait_pipe(int *fd)
{
    close(fd[0]);
    close(fd[1]);
}

//字符转整型
long int CtoI32(unsigned char *str, int addr, int p)
{
    addr *= p;
    return (str[0 + addr] | str[1 + addr] << 8 | str[2 + addr] << 16 | str[3 + addr] << 24);
}

//整型转字符
void ItoC32(unsigned char *str, long int num, int addr, int p)
{
    addr *= p;
    str[0 + addr] = num;
    str[1 + addr] = num >> 8;
    str[2 + addr] = num >> 16;
    str[3 + addr] = num >> 24;
}

//主程序
int main(int argc, char const *argv[])
{
    pid_t pid[3],MD5_pid;  
    int res;
    int End = 2;
    int shmid;
    char *shmptr;
    struct timeval start,finish;
    long int temp1 = 68273874,temp2 = 0;

    gettimeofday(&start,NULL);//获取程序开始时间

    //检查参数
    if (argc != 3) 
    {
        fprintf(stderr, "Wrong number of parameters: %d != 3\n", argc);  
        exit(EXIT_FAILURE);
    }

    //管道是否存在以及创建有名管道FIFO
    if((mkfifo(FIFO_path, FILE_MODE)) < 0 && (errno != EEXIST))
        Error_Exit("Can not creat FIFO\n");

    //创建共享存储区
    if((shmid = shmget(IPC_PRIVATE,SHM_SIZE,SHM_MODE)) < 0)
        Error_Exit("Shmget error\n");
    if((shmptr = shmat(shmid, 0, 0)) == (void *)-1)
        Error_Exit("shmat error\n");
    
    //创建生产者进程
    if((pid[1] = fork()) < 0)
        Error_Exit("fork1 error\n");
        
    //生产者进程
    else if(pid[1] == 0)
    {
        int i;
        int Source_fd;
        int Pro_fd;
        int Pro_FIFO_fd;
        pid_t Pro_pid[Pro_child_Num + 1];
        int Pro_End = Pro_child_Num;
        static int Pro_Pipe_fd[2], Pro_Pipe_end_fd[2];
        char Pro_chr;

        //共享存储区清空
        ItoC32(shmptr,0,0,4);
        
        //创建无名管道
        Tell_Wait_pipe(Pro_Pipe_fd,1);
        Tell_Wait_pipe(Pro_Pipe_end_fd,0);

        //读打开源文件
        printf("--Producer<%d> created successfully\n--Open source file <%s>\n", getpid(), argv[1]);
        if ((Pro_fd = open(argv[1], O_RDONLY)) == -1)
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
                long int Pro_bytes,Pro_bytes_sum;
                char Pro_buffer[BUFFER_SIZE];
                off_t file_position,file_position2;
                
                printf("----Producer_child[%d]<%d> created successfully\n", i, getpid());
                Pro_bytes = 0;
                while(1)
                {
                    //互斥
                    Wait_pipe(Pro_Pipe_fd,1);

                    file_position = lseek(Pro_fd,0,SEEK_CUR);
                    if((Pro_count = read(Pro_fd, Pro_buffer, BUFFER_SIZE - 4)) == -1)
                        Error_Exit("Read source file error\n");
                    if(Pro_count == 0)  //文件为空时 返回0
                    {
                        Tell_pipe(Pro_Pipe_fd,1);
                        break;
                    }
                    ItoC32(Pro_buffer,file_position,Pro_count,1); //将文件偏移量写入buf
                    printf("Pro_child<%d> fread %d bytes file_position = %ld \n", i, Pro_count, file_position);
                    Pro_bytes_sum = CtoI32(shmptr,0,4); //从共享区0取数据
                    Pro_bytes_sum += Pro_count;
                    ItoC32(shmptr,Pro_bytes_sum,0,4);//写入共享区0

                    Tell_pipe(Pro_Pipe_fd,1);

                    if(Pro_count == BUFFER_SIZE - 4)
                    {
                        if(write(Pro_FIFO_fd, Pro_buffer, Pro_count + 4) == -1)
                            Error_Exit("Write FIFO error\n");
                        Pro_bytes += Pro_count;
                    }
                    else  //到达文件尾端，Pro_count都可能小于BUFFER_SIZE - 4
                    {
                        Wait_pipe(Pro_Pipe_end_fd,Pro_child_Num - 1);
                        if(write(Pro_FIFO_fd, Pro_buffer, Pro_count + 4) == -1)
                            Error_Exit("Write FIFO error\n");
                        Pro_bytes += Pro_count;
                        file_position = lseek(Pro_fd,0,SEEK_CUR);
                        printf("Pro_child<%d> read file finish, file_position = %ld\n", i, file_position);
                        break;
                    }
                }
                Tell_pipe(Pro_Pipe_end_fd,1);
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
        close(Pro_fd);
        Close_Tell_Wait_pipe(Pro_Pipe_end_fd);
        Close_Tell_Wait_pipe(Pro_Pipe_fd);
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
        static int Cus_Pipe_fd[2],Cus_Pipe_end_fd[2];
        char Cus_chr;

        //共享存储区清空
        ItoC32(shmptr,0,1,4);

        //创建无名管道
        Tell_Wait_pipe(Cus_Pipe_fd,1);
        Tell_Wait_pipe(Cus_Pipe_end_fd,0);
        
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
                long int Cus_bytes, Cus_bytes_sum;
                char Cus_buffer[BUFFER_SIZE + 1];
                long int file_position,write_position;
                printf("----Customer_child[%d]<%d> created successfully\n", j, getpid());
                Cus_bytes = 0;
                while(1)
                {
                    if((Cus_count = read(Cus_FIFO_fd, Cus_buffer, BUFFER_SIZE)) == -1)
                        Error_Exit("Read FIFO error\n");
                    if(Cus_count == 0)
                    {
                        if(errno != EAGAIN)
                            break;
                    }
                    else
                    {
                        if(Cus_count  == BUFFER_SIZE)
                        {
                            Wait_pipe(Cus_Pipe_fd,1);
                            file_position = ftell(Cus_fp);
                            write_position = CtoI32(Cus_buffer,Cus_count - 4,1);
                            printf("-------------------------Cus_child<%d> read %d bytes ,now_position = %ld, write_position = %ld\n",j,Cus_count - 4,file_position,write_position);
                            fseek(Cus_fp,write_position,SEEK_SET);
                            fwrite(Cus_buffer,sizeof(char),Cus_count - 4,Cus_fp);
                            Cus_bytes_sum = CtoI32(shmptr,1,4);
                            Cus_bytes_sum += (Cus_count - 4);
                            ItoC32(shmptr,Cus_bytes_sum,1,4);
                            //必须冲洗缓冲区，不然多进程读会出现在缓冲区数据的次序不一致
                            if(fflush(Cus_fp) != 0)
                                Error_Exit("fflush error\n");
                            Tell_pipe(Cus_Pipe_fd,1);
                            Cus_bytes += (Cus_count - 4);
                        }
                        else
                        {
                            Wait_pipe(Cus_Pipe_end_fd,Cus_child_Num - 1);
                            file_position = ftell(Cus_fp);
                            write_position = CtoI32(Cus_buffer,Cus_count - 4,1);
                            printf("-------------------------Cus_child<%d> read fifo finish, now_position = %ld, write_position = %ld\n",j,file_position,write_position);
                            fseek(Cus_fp,write_position,SEEK_SET);
                            fwrite(Cus_buffer,sizeof(char),Cus_count - 4,Cus_fp);
                            Cus_bytes_sum = CtoI32(shmptr,1,4);
                            Cus_bytes_sum += (Cus_count - 4);
                            ItoC32(shmptr,Cus_bytes_sum,1,4);
                            //必须冲洗缓冲区，不然多进程读会出现在缓冲区数据的次序不一致
                            if(fflush(Cus_fp) != 0)
                                Error_Exit("fflush error\n");
                            Cus_bytes += (Cus_count - 4);
                            break;
                        }
                    }
                }
                Tell_pipe(Cus_Pipe_end_fd,1);
                printf("----Customer_child[%d]<%d> have read %ld bytes \n", j, getpid(), Cus_bytes);
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
        Close_Tell_Wait_pipe(Cus_Pipe_end_fd);
        Close_Tell_Wait_pipe(Cus_Pipe_fd);
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
    printf("Producer write %ld KB\n", CtoI32(shmptr,0,4));
    printf("Customer read %ld KB\n", CtoI32(shmptr,1,4));

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


