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
void StrtoHex(unsigned char *str_o, unsigned char *str_d)
{
    unsigned char len = strlen(str_o);
    int i = 0, j = 0;
    int high_sta = 1;
    int temp;

    while(i < len)
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
    str_d[j] = '\0';
    //printf("\n");
}

//hex流转字符串
void HextoStr(unsigned char *str_o, int len, unsigned char *str_d)
{
    int i = 0, j = 0;
    int high_sta = 1;
    unsigned char temp;

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

//获取回传数据的大小
//清除hex数据大小信息，并将hex数据大小状态字返回
int Get_len(unsigned char *str, int *len)
{
    unsigned char sta;
    int i;

    (*len) = str[0] & 0x0f;
    sta = (str[0] & 0xf0) >> 4;

    for(i = 0; i < (*len); i++)
        str[i] = str[i + 1];

    str[i] = '\0';

    return sta;
}

//设置发送数据的hex大小信息
//将hex数据的大小信息加入首字节
void Set_len(unsigned char *str)
{
    int len = strlen(str);
    
    for(int i = len; i > 0; i--)
        str[i] = str[i - 1];

    str[0] = 0xc0 | len;
}

//发送指令
//成功返回发送的字节数，失败返回-1
int Send_Cmd(unsigned char *str, int len)
{
    unsigned char Send_str[16];
    int send_len;

    StrtoHex(str, Send_str);                //字符串转换为hex
    Set_len(Send_str);                      //加入hex长度信息
    send_len = Send_Date(Send_str,16);      //发送数据
    if(send_len < 0) 
        return -1;
    else
        return send_len;
}

/*                      回传数据格式                       
                        0           1-14      15
回传数据小于16字节：
                    (0xc0 + len)    date    state
回传数据大于16字节：     
                        0x8f             date           起始
                        0X0f             date           中间
                    (0x40 + len)    date    state       结束
*/
//接收数据，返回最终收到的完整hex数据（最后一字节包含命令执行的状态码）
//成功返回实际回传有效数据的字节数
//接收失败返回-1，存储数据的容量不够返回-2
int Recv_Value(unsigned char *str, int len)
{
    unsigned char Recv_str[16];
    unsigned char str_temp[128];
    unsigned char sta, recv_sta;;
    int recv_len = 0;
    int Value_len;

    while(1)
    {
        recv_sta = Recv_Date(Recv_str,16);  //读取16字节数据
        if(recv_sta < 0) 
            return -1;

        sta = Get_len(Recv_str, &Value_len);//获取回传hex数据大小状态字
        recv_len += Value_len;
        if(sta == 8 || sta == 12)           //0x8f或0xcx 的首字节，表示收到的第一组数据
            strcpy(str_temp, Recv_str);

        else if(sta == 4 || sta ==0)        //0x4x或0x0f 的首字节，表示收到的最后一组或中间组的数据
            strcat(str_temp, Recv_str);

        if(sta == 12 || sta == 4)           //0xc0表示只有一组，0x4x表示最后一组，因此收到就停止接收
            break;
    }

    if(strlen(str_temp) > len)        
        return -2;
    else
        strcpy(str, str_temp);

    return recv_len;
}

//返回数据中最后一个字节是状态码,将接收数据变为纯数据
//获取返回数据中的指令执行状态码，并清除该状态码
//返回-1，指令执行失败；  返回-2，指令不支持；  返回0，指令执行成功
int Get_Cmd_Sta(unsigned char *Recv_str)
{
    int str_len;

    str_len = strlen(Recv_str);
    if(Recv_str[0] == 0x6a && str_len == 1)
    {
        Recv_str[0] = '\0';
        return -1;
    }
    else if(Recv_str[0] == 0x6d && str_len == 1)
    {
        Recv_str[0] = '\0';
        return -2;
    }
    else if(Recv_str[str_len - 1] == 0x90)
    {
        Recv_str[str_len - 1] = '\0';
        return 0;
    }
    return -2;
}

//获取随机数
//成功返回读取的有效hex数据字节数
//发送指令失败返回-1，接收数据失败返回-2，指令执行失败返回-3
int Get_Radom(unsigned char *str)
{
    int length;
    int ret;
    
    ret = Send_Cmd("01",16);
    
    if(ret < 0)
        return -1;

    length = Recv_Value(str,17);
    if(length < 0)
        return -2;

    if(Get_Cmd_Sta(str) < 0)
        return -3;

    return length - 1;
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

//生成公私钥对,48字节公钥，24字节私钥
//成功返回读取的有效hex数据字节数
//发送指令失败返回-1，接收数据失败返回-2，指令执行失败返回-3
int Generate_Key_Pair(unsigned char *Public_key, unsigned char *Private_key)
{
    int length;
    int ret,i;
    unsigned char Recv_str[73];
    
    ret = Send_Cmd("28",16);
    
    if(ret < 0)
        return -1;

    length = Recv_Value(Recv_str,73);
    if(length < 0)
        return -2;

    if(Get_Cmd_Sta(Recv_str) < 0)
        return -3;
    
    for(i = 0; i < 72; i++)
    {
        if(i < 48)
            Public_key[i] = Recv_str[i];
        else
            Private_key[i-48] =  Recv_str[i];
    }

    return length - 1;
}


