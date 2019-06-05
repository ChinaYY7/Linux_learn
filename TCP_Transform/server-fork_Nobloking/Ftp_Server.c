#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "Ftp_ServerUtility.h"
#include "Trans_Protocol.h"


int main(int argc, char *argv[])
{
    char service[SERVICE_LEN];
    
    //从命令行或者配置文件获取需要绑定的端口号
    Get_Server_Config(service,argc,argv);
    
    //创建服务端监听TCP套接字
    fputc('\n',stdout);
    int servSock = SetupTCPServerSocket(service);  
    if(servSock < 0)
        System_Error_Exit("SetupTCPServerSocket() faild");

    //监听套接字设置为非阻塞
    if (fcntl(servSock, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put servSock into non-blocking");

    int clntSock;
    unsigned int childProcCount = 0;
    pid_t processID;
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    int epfd,nfds;
    struct epoll_event ev,events[5]; //ev用于注册事件，数组用于返回要处理的事件
    epfd = epoll_create(1); //只需要监听一个描述符——标准输入
    ev.data.fd = servSock;
    ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, servSock, &ev); //注册epoll事件

    while(1)
    {
        nfds = epoll_wait(epfd, events, 5, -1);
        printf("Detecte event: %d\n",nfds);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == servSock)
            {
                while (1)
                {
                    clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
                    if(clntSock < 0)
                    {
                        if(errno != EWOULDBLOCK)
                            System_Error_Exit("accept faild!");
                        else
                            break;
                    }
                    processID = fork();
                    if(processID < 0)
                        System_Error_Exit("fork() faild");
                    else if(processID == 0)
                    {
                        close(servSock);
                        printf("\nProcessID: %d\nClient->Server: ",getpid());
                        Get_Peer_Name(clntSock);
                        HandleTCPClient(clntSock);
                        exit(0);
                    }
                    close(clntSock);
                    childProcCount++;
                    if(childProcCount > 0)
                        Clean_Zombies_Process(&childProcCount);
                }
            }
        }
    }
}