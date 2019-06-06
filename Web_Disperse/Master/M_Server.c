#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "M_ServerUtility.h"
#include "Trans_Protocol.h"


int main(int argc, char *argv[])
{
    char Chrom_Port[SERVICE_LEN];
    char Slave_Port[SERVICE_LEN];
    
    //从命令行或者配置文件获取需要绑定的端口号
    Get_Server_Config(Chrom_Port,Slave_Port,argc,argv);
    
    //创建主服务器监听浏览器的TCP套接字
    fputc('\n',stdout);
    printf("The address for Chrom connecting:\n");
    int Chrom_Sock_lisen = SetupTCPServerSocket(Chrom_Port);  
    if(Chrom_Sock_lisen < 0)
        System_Error_Exit("SetupTCPServerSocket() faild");

    //设置为非阻塞
    if (fcntl(Chrom_Sock_lisen, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Chrom_Sock_lisen into non-blocking");

    //创建主服务器监听子服务器的TCP套接字
    fputc('\n',stdout);
    printf("The address for slave server connecting:\n");
    int Slave_Sock_lisen = SetupTCPServerSocket(Slave_Port);  
    if(Slave_Sock_lisen < 0)
        System_Error_Exit("SetupTCPServerSocket() faild");

    //设置为非阻塞
    if (fcntl(Slave_Sock_lisen, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Slave_Sock_lisen into non-blocking");

    int Chrom_Sock;
    int Slave_Sock;
    int Temp_Sock;
    unsigned int childProcCount = 0;
    pid_t processID;
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    int epfd,nfds;
    struct epoll_event ev,events[20]; //ev用于注册事件，数组用于返回要处理的事件
    epfd = epoll_create(1); //只需要监听一个描述符——标准输入

    ev.data.fd = Chrom_Sock_lisen;
    ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, Chrom_Sock_lisen, &ev); //注册epoll事件

    ev.data.fd = Slave_Sock_lisen;
    ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, Slave_Sock_lisen, &ev); //注册epoll事件

    while(1)
    {
        nfds = epoll_wait(epfd, events, 20, -1);
        printf("Detecte event: %d\n",nfds);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == Slave_Sock_lisen)
            {
                Slave_Sock = accept(Slave_Sock_lisen, (struct sockaddr *)&clntAddr, &clntAddrLen);
                if(Slave_Sock < 0)
                {
                    if(errno != EWOULDBLOCK)
                        System_Error_Exit("Slave_Sock accept faild!");
                }

                struct sigaction handler;

                handler.sa_handler = SIG_IGN;
                if(sigfillset(&handler.sa_mask) < 0)
                    System_Error_Exit("sigfillset() failed");

                handler.sa_flags = 0;

                if(sigaction(SIGPIPE,&handler, 0) < 0)
                    System_Error_Exit("sigaction() faild for SIGPIPE");

                    
                printf("A new Slave_Server Connected\n");
                ev.data.fd = Slave_Sock;
                ev.events = EPOLLIN | EPOLLET; 
                epoll_ctl(epfd, EPOLL_CTL_ADD, Slave_Sock, &ev);
            }
            else if(events[i].data.fd == Chrom_Sock_lisen)
            {
                Chrom_Sock = accept(Chrom_Sock_lisen, (struct sockaddr *)&clntAddr, &clntAddrLen);
                if(Chrom_Sock < 0)
                {
                    if(errno != EWOULDBLOCK)
                        System_Error_Exit("Chrom_Sock accept faild!");
                }
                Reponse_Chrom(Chrom_Sock);
                    /*
                    processID = fork();
                    if(processID < 0)
                        System_Error_Exit("fork() faild");
                    else if(processID == 0)
                    {
                        close(Chrom_Sock_lisen);
                        printf("\nProcessID: %d\nClient->Server: ",getpid());
                        Get_Peer_Name(Chrom_Sock);
                        HandleTCPClient(Chrom_Sock);
                        exit(0);
                    }
                    close(Chrom_Sock);
                    childProcCount++;
                    if(childProcCount > 0)
                        Clean_Zombies_Process(&childProcCount);
                    */
            }
            else if(events[i].events&EPOLLIN)
            {
                if ( (Temp_Sock = events[i].data.fd) < 0)
                    continue;
                Talk_Slave(Temp_Sock); 
            }
        }
    }
}