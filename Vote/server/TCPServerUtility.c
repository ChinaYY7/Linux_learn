#include "TCPServerUtility.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "AddressUtility.h"
#include "VoteProtocol.h"
#include "VoteCoding.h"
#include "Framer.h"

int SetupTCPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));

    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    
    if(rtnVal != 0)
        Deal_User_Error("getaddrinfo()", "faild", ERROR_VALUE);
    
    int servSock = -1;

    for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(servSock < 0)
            continue;
        
        if((bind(servSock,addr->ai_addr,addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0))
        {
            fputs("Binding to ",stdout);
            PrintSockAddress(addr->ai_addr,stdout);
            break;
        }
        
        close(servSock);
        servSock = -1;
    }

    freeaddrinfo(servAddr);

    return servSock;
}

int Get_Sock_Name(int sock_fd)
{
    struct sockaddr_storage localAddr;
    socklen_t addrSize = sizeof(localAddr);
    if(getsockname(sock_fd, (struct sockaddr *)&localAddr,&addrSize) < 0)
        Deal_System_Error("getsockname() faild!", ERROR_VALUE);

    PrintSockAddress((struct sockaddr *)&localAddr, stdout);
}

int AcceptTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    int clntScok = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    
    if(clntScok < 0)
        Deal_System_Error("accept() faild!", ERROR_VALUE);
    
    fputs("\nHandling client ", stdout);
    PrintSockAddress((struct sockaddr *)&clntAddr, stdout);
    return clntScok;
}

int TCP_nSend(int sock_fd, const void *buf, size_t buf_len)
{
    ssize_t Send_Bytes;

    Send_Bytes = send(sock_fd, buf, buf_len, 0);
    if(Send_Bytes < 0)
        Deal_System_Error("send faild", ERROR_VALUE);
    else if (Send_Bytes != buf_len)
        Deal_User_Error("send","sent unexpected number of bytes", ERROR_VALUE);

    return Send_Bytes;
}

int TCP_nReceive(int sock_fd, void *buf, size_t buf_len)
{
    ssize_t numBytes;
    unsigned int totalBytesRcvd = 0;

    if(buf_len == 0)
    {
        numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
        if(numBytes < 0)
            Deal_System_Error("recv() faild\n", ERROR_VALUE);
        else if(numBytes == 0)
            printf("\nconnection closed prematurely\n");

        return numBytes;
    }
    else
    {
        while(totalBytesRcvd < buf_len)
        {
            numBytes = recv(sock_fd, buf, BUFFSIZE - 1, 0);
            if(numBytes < 0)
                Deal_System_Error("recv() faild\n", ERROR_VALUE);
            else if(numBytes == 0)
                Deal_User_Error("recv", "connection closed prematurely", ERROR_VALUE);
            totalBytesRcvd += numBytes;
        }

        return totalBytesRcvd;
    }   
}

static uint64_t counts[MAX_CANDIDATE + 1];

void HandleTCPClient(int clntSocket)
{
    FILE  *Channel = fdopen(clntSocket, "r+");
    
    if(Channel == NULL)
        Deal_System_Error("fdopen() faild", ERROR_VALUE);
    
    int msize;
    uint8_t inbuf[MAX_WIRE_SIZE];
    VoteInfo v;

    printf("\n------------Process-----------\n");
    while((msize =  GetNextMsg(Channel, inbuf, MAX_WIRE_SIZE)) > 0)
    {
        //printf("inBuf: %s\n ",inbuf);
        memset(&v, 0, sizeof(v));
        printf("Received message (%d bytes) \n", msize);
        if(Decode(&v,inbuf,msize))
        {
            if(v.isResponse == False)
            {
                v.isResponse = True;
                if(v.candidate >= 0 && v.candidate <= MAX_CANDIDATE)
                {
                    if(v.isInquiry == False)
                        counts[v.candidate] += 1;
                    v.count = counts[v.candidate];
                }
            }

            uint8_t outBuf[MAX_WIRE_SIZE];
            msize = Encode(&v, outBuf, MAX_WIRE_SIZE);
            //printf("outBuf: %s\n ",outBuf);
            if(PutMsg(outBuf,msize,Channel) < 0)
                Deal_User_Error("PutMsg(outbuf)","faild !", ERROR_VALUE);
            else
            {
                printf("-------------END--------------\n");
                printf("Processed %s for candidate %d;\nCurrent count is %lu\n",(v.isInquiry ? "inquiry" : "vote"), v.candidate, v.count);
            }

            fflush(Channel);
        }
        else
        {
            fputs("Parse error, closing connection.\n", stderr);
            break;
        }      
    }

    puts("Client finished");
    fclose(Channel);
}