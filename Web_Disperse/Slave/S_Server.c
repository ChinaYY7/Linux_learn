#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "apue.h"
#include "TCP_Socket.h"
#include "Deal_Error.h"
#include "S_ServerUtility.h"
#include "Trans_Protocol.h"

int main(int argc, char *argv[])
{
    char server[SERVER_LEN], service[SERVICE_LEN];
    
    //从命令行或者配置文件获取服务器地址和端口号
    Get_Server_Config(server, service, argc, argv);
    
    //创建子服务器-主服务器连接套接字
    printf("\nSlave_Server->Master_Server: ");
    int Master_sock = SetupTCPClientSocket(server, service);
    if (Master_sock < 0)
        User_Error_Exit("SetupTCPClientSocket(Master_sock) faild!", "Unable to connect");

    //设置为非阻塞
    if (fcntl(Master_sock, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Master_sock into non-blocking");

    //创建子服务器监听浏览器的TCP套接字
    char Browser_Port[6];
    sprintf(Browser_Port,"%d",getpid() % 5000 + 6000);
    int Browser_Sock_lisen = SetupTCPServerSocket(Browser_Port);  
    if(Browser_Sock_lisen < 0)
        System_Error_Exit("SetupTCPServerSocket(Browser_Sock_lisen) faild");

    //设置为非阻塞
    if (fcntl(Browser_Sock_lisen, F_SETFL, O_NONBLOCK) < 0)
        System_Error_Exit("Unable to put Browser_Sock_lisen into non-blocking");

    //使用流包装套接字
    FILE *Master_Str = fdopen(Master_sock, "r+");
    if(Master_Str == NULL)
        System_Error_Exit("fdopen() faild");

    //给服务器传递连接的端口
    Send_Messege(Master_Str, Browser_Port);

    int Browser_Sock;
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    int epfd,nfds;
    struct epoll_event ev,events[20];                       //ev用于注册事件，数组用于返回要处理的事件
    epfd = epoll_create(1);                                 //只需要监听一个描述符——标准输入

    ev.data.fd = Browser_Sock_lisen;
    ev.events = EPOLLIN | EPOLLET;                          //监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, Browser_Sock_lisen, &ev);  //注册epoll事件

    while(1)
    {
        nfds = epoll_wait(epfd, events, 20, -1);
        //printf("Detecte event: %d\n",nfds);
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == Browser_Sock_lisen)     //浏览器请求连接
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

                Get_Peer_Name(Browser_Sock);
                printf(" Browser->Client\n");
                Reponse_Browser(Browser_Sock, Browser_Port);              //回应浏览器
            }
        }
    }
    fclose(Master_Str);
    return 0;
} 