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
    printf("The address for Browser connecting:\n");
    int Browser_Sock_lisen = SetupTCPServerSocket(Chrom_Port);  
    if(Browser_Sock_lisen < 0)
        System_Error_Exit("SetupTCPServerSocket() faild");

    //设置为非阻塞
    if (fcntl(Browser_Sock_lisen, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Browser_Sock_lisen into non-blocking");

    //创建主服务器监听子服务器的TCP套接字
    fputc('\n',stdout);
    printf("The address for slave server connecting:\n");
    int Slave_Sock_lisen = SetupTCPServerSocket(Slave_Port);  
    if(Slave_Sock_lisen < 0)
        System_Error_Exit("SetupTCPServerSocket() faild");

    //设置为非阻塞
    if (fcntl(Slave_Sock_lisen, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Slave_Sock_lisen into non-blocking");

    int Browser_Sock;
    int Slave_Sock;
    int Temp_Sock;
    int State;
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    int epfd,nfds;
    struct epoll_event ev,events[20]; //ev用于注册事件，数组用于返回要处理的事件
    epfd = epoll_create(1); //只需要监听一个描述符——标准输入

    ev.data.fd = Browser_Sock_lisen;
    ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, Browser_Sock_lisen, &ev); //注册epoll事件

    ev.data.fd = Slave_Sock_lisen;
    ev.events = EPOLLIN | EPOLLET; //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, Slave_Sock_lisen, &ev); //注册epoll事件

    Init_Slave_Server_Address_Tbl();

    Shield_SIGPIPE();

    while(1)
    {
        nfds = epoll_wait(epfd, events, 20, -1);
        //printf("Detecte event: %d\n",nfds);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == Slave_Sock_lisen)       //子服务器请求连接
            {
                Slave_Sock = accept(Slave_Sock_lisen, (struct sockaddr *)&clntAddr, &clntAddrLen);
                if(Slave_Sock < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        System_Error("Slave_Sock accept faild!");
                        continue;
                    }
                }

                printf("A new Slave_Server Connected\n");
                ev.data.fd = Slave_Sock;
                ev.events = EPOLLIN | EPOLLET; 
                epoll_ctl(epfd, EPOLL_CTL_ADD, Slave_Sock, &ev);
            }
            else if(events[i].data.fd == Browser_Sock_lisen)  //浏览器请求连接
            {
                Browser_Sock = accept(Browser_Sock_lisen, (struct sockaddr *)&clntAddr, &clntAddrLen);
                if(Browser_Sock < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        System_Error("Browser_Sock accept faild!");
                        continue;
                    }     
                }
                Reponse_Browser(Browser_Sock);                  //转发连接，http重定向
            }
            else if(events[i].events&EPOLLIN)               //子服务器有数据到来
            {
                if ( (Temp_Sock = events[i].data.fd) < 0)
                    continue;
                State = Talk_Slave(Temp_Sock);
                if(State == 1)
                {
                    ev.data.fd = Temp_Sock;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, Temp_Sock, &ev);
                } 
            }
        }
    }
}