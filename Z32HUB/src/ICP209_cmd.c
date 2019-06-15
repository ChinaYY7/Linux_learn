#include "ICP209_cmd.h"
#include "usb_hub.h"

//ctrl+c 信号处理函数
void sigint_handler(int sig)
{
    printf("\nZ32HUB Exit!!!\n");	
    exit(0);
}

//ctrl+c 信号处理
void Signal_Deal_Ctc(void)
{
    //修改ctrl+c 信号的处理
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}

//字符串转hex流
void StrToByte_stream(unsigned char *str_o, unsigned char *str_d)
{
    unsigned char len = strlen(str_o);
    int i = 0, j = 1;
    int high_sta = 1;
    int temp;

    //printf("%s thransfer:\n", str_o);
    //获得实际16进制表示时，字节数（两个字符表示一个16进制码），命令第一个字节为该命令有多少字节
    str_d[0] = 0xc0 | (len / 2); 
    //printf("%x ", str_d[0]);

    while(i < 32)
    {
        if(high_sta)
        {
            if(str_o[i] < 58)           //字符为0-9
                temp = str_o[i]-48;     
            else if(str_o[i] < 91)      //字符为A-Z
                temp = str_o[i]-55;
            else if(str_o[i] < 123)     //字符为a-z
                temp = str_o[i]-87;

            str_d[j] = (temp << 4) & 0xf0;
            i++;
            high_sta = 0;
        }
        else
        {
            if(str_o[i] < 58)
                temp = str_o[i]-48;
            else if(str_o[i] < 91)
                temp = str_o[i]-55;
            else if(str_o[i] < 123)
                temp = str_o[i]-87;

            str_d[j] |= (temp & 0x0f);
            //printf("%02x ", str_d[j]);
            j++;
            i++;
            high_sta = 1;
        }
    }
    //printf("\n");
}

//hex流转字符串
void Byte_streamToStr(unsigned char *str_o, unsigned char *str_d)
{
    unsigned char len ;
    int i = 0, j = 1;
    int high_sta = 1;
    unsigned char temp;

    len = str_o[0] & 0x0f;
    //printf("len = %d\n", len);

    while(i < len * 2)
    {
        if(high_sta)
        {
            temp = (str_o[j] >> 4) & 0x0f;
            if(temp < 10)               //hex为0-9
                str_d[i] = temp + 48;
            else                        //hex为a-f
                str_d[i] = temp + 87;
            i++;
            high_sta = 0;
        }
        else
        {
            temp = str_o[j] & 0x0f;
            if(temp < 10)
                str_d[i] = temp + 48;
            else
                str_d[i] = temp + 87;
            i++;
            j++;
            high_sta = 1;
        }
    }
    str_d[i] = '\0';
}

//发送指令
//成功返回发送的字节数，失败返回-1
int Send_Cmd(unsigned char *str, int len)
{
    unsigned char Send_str[16];
    int send_len;

    StrToByte_stream(str, Send_str);
    send_len = Send_Date(Send_str,16);
    if(send_len < 0) 
        return -1;
    else
        return send_len;
}

//接收数据
//成功返回发送的字节数，失败返回-1
int Recv_Value(unsigned char *str, int len)
{
    unsigned char Recv_str[16];
    int recv_len;

    recv_len = Recv_Date(Recv_str,16);
    if(recv_len < 0) 
        return -1;

    Byte_streamToStr(Recv_str, str);

    return recv_len;
}

//获取返回数据中的指令执行状态码，并清除该状态码
//返回-1，指令执行失败；  返回-2，指令不支持；  返回0，指令执行成功
int Get_Cmd_Sta(unsigned char *Recv_str)
{
    int str_len;

    str_len = strlen(Recv_str);
    if(Recv_str[0] == '6' && Recv_str[1] == 'a' && str_len == 2)
    {
        return -1;
        Recv_str[str_len - 2] = '\0';
    }
        
    else if(Recv_str[0] == '6' && Recv_str[1] == 'd' && str_len == 2)
    {
        Recv_str[str_len - 2] = '\0';
        return -2;
    }
    else if(Recv_str[str_len - 2] == '9' && Recv_str[str_len - 1] == '0')
    {
        Recv_str[str_len - 2] = '\0';
        return 0;
    }
}

//获取随机数
//成功返回读取的字节数
//发送指令失败返回-1，接收数据失败返回-2，指令执行失败返回-3
int Get_Radom(unsigned char *str)
{
    int length;
    int ret;
    unsigned char Recv_str_tmp[64];
    
    ret = Send_Cmd("01",16);
    if(ret < 0)
        return -3;
    
    ret = Recv_Value(Recv_str_tmp,16);
    if(ret < 0)
        return -2;
    strcpy(str,Recv_str_tmp);
    length = ret;
            
    ret = Recv_Value(Recv_str_tmp,16);
    if(ret < 0)
        return -2;
    strcat(str, Recv_str_tmp);
    length+=ret;

    if(Get_Cmd_Sta(str) < 0)
        return -1;

    return length;
}

//验证PIN
//成功返回0
//发送指令失败返回-1，接收数据失败返回-2，指令执行失败返回-3
int Verfiy_PIN(unsigned char *pin)
{
    int ret;
    unsigned char Recv_str[64];

    ret = Send_Cmd(pin,16);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    if(Get_Cmd_Sta(Recv_str) < 0)
        return -3;
    else
        return 0;
}

