#include "apue.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Deal_Error.h"
#include "TCPClientUtility.h"
#include "VoteProtocol.h"
#include "VoteCoding.h"
#include "Framer.h"

#define SERVER_LEN 100
#define SERVICE_LEN 100
#define CONFIG_PATH "./config/connect.config"
#define DELIM " "

int main(int argc, char *argv[])
{
    FILE *Config_file_fp;
    char addrBuffer[SERVER_LEN];
    char server[SERVER_LEN], service[SERVICE_LEN];
    char *token;

    int candidateID;
    Bool inq;

    if(argc ==5)
    {
        strcpy(server, argv[1]);
        strcpy(service, argv[2]);
        candidateID = atoi(argv[3]);
        if(strcmp(argv[4], INQSTR) == 0)
            inq = True;
        else
            inq = False;
    }
    else if(argc == 3)
    {
        if((Config_file_fp = fopen(CONFIG_PATH,"r")) == NULL)
            Deal_User_Error("fopen()", "connect.config not exit!", ERROR_VALUE);

        fgets(addrBuffer, SERVER_LEN, Config_file_fp);
        token = strtok(addrBuffer, DELIM);
        strcpy(server,token);

        token = strtok(NULL, DELIM);
        strcpy(service,token);

        candidateID = atoi(argv[1]);
        if(strcmp(argv[2], INQSTR) == 0)
            inq = True;
        else
            inq = False;
    }
    else
        Deal_User_Error("wrong arguments","<Server> <Port/Service> <Candidate> [I]", ERROR_VALUE);

    if(candidateID < 0 || candidateID > MAX_CANDIDATE)
        Deal_User_Error("CandidateID invalid","",ERROR_VALUE);

    int sock = SetupTCPClientSocket(server, service);

    if (sock < 0)
        Deal_User_Error("SetupTCPClientSocket() faild!", "Unable to connect", ERROR_VALUE);

    FILE *str = fdopen(sock, "r+");
    if(str == NULL)
        Deal_System_Error("fdopen() faild", ERROR_VALUE);

    VoteInfo vi;
    memset(&vi, 0, sizeof(vi));

    vi.isInquiry = inq;
    vi.candidate = candidateID;
    vi.isResponse = False;

    uint8_t outbuf[MAX_WIRE_SIZE];
    size_t reqsize = Encode(&vi, outbuf, MAX_WIRE_SIZE);

    printf("Sending %ld-byte %s for candidate %d ... \n", reqsize, (inq ? "inquiry" : "vote"), candidateID);
    //printf("outBuf: %s\n ",outbuf);

    if(PutMsg(outbuf,reqsize,str) < 0)
        Deal_User_Error("PutMsg(outbuf)","faild !", ERROR_VALUE);
    
    uint8_t inbuf[MAX_CANDIDATE];
    
    reqsize = GetNextMsg(str,inbuf,MAX_WIRE_SIZE);
    //printf("inBuf: %s\n ",inbuf);

    if(Decode(&vi, inbuf, reqsize))
    {
        printf("Received:\n");
        if(vi.isResponse)
            printf("Response to ");
        if(vi.isInquiry)
            printf("inquiry ");
        else
            printf("vote ");
        
        printf("for candidate %d\n", vi.candidate);
        if(vi.isResponse)
            printf("count = %lu\n", vi.count);
    }

    fclose(str);

    return 0;
} 