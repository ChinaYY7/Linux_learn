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

//功能：字符串转hex流
//入口参数：str_o:原字符串 str_d：转换结果
//返回值：转后后hex流的大小（hex流不能当作字符串处理，因为'\0'的hex为0x00）
int StrtoHex(unsigned char *str_o, unsigned char *str_d)
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

    return len / 2;
}

//功能：hex流转字符串
//入口参数：str_o:原hex流 str_d：转换结果
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

/*                        发送数据格式                       
                        0           1-15
发送数据小于16字节：
                    (0xc0 & len)    date  
回传数据大于16字节：     
                        0x8f        date           起始
                        0X0f        date           中间
                    (0x40 & len)    date           结束
*/

//功能：将hex数据的大小信息加入hex流首字节
//入口参数：str_o:原hex流，str_d：转换结果，len：hex流大小，count指令截取位置
//返回值：下一次截取位置，若为0，表示最后一次截取
int Set_len(unsigned char *str_o, unsigned char *str_d, int len, int count)
{
    int differ = len - count;
    int i;
    if(count < len)
    {
        if(differ == len)               //第一次截取
        {
            if(differ < 16)             //指令总长度小于16字节
                str_d[0] = 0xc0 | len;
            else
                str_d[0] = 0x8f;
        }
        else                            //中间截取
        {
            if(differ < 16)             //剩余待截取数据小于16，数据将要拆分完成
                str_d[0] = 0x40 | differ;
            else
                str_d[0] = 0x0f;
        }
        
        for(i = count; i < count + 15; i++)
        {
            if(i >= len)
            {
                str_d[i - count + 1] = '\0';
                return 0;
            }
            str_d[i - count + 1] = str_o[i];
        }
        if(count + 15 == len)
            return 0;
        return count + 15;        
    }
    else
        return 0;
}

//功能：发送指令
//入口参数：指令字符串
//返回值：成功：发送的数据大小， 失败：-1
int Send_Cmd(unsigned char *str)
{
    unsigned char Send_str_tmp[128];
    unsigned char Send_str[17];
    int send_len = 0, hex_len;
    int count = 0;
    int i = 0;

    hex_len = StrtoHex(str, Send_str_tmp);                //字符串转换为hex
    while(1)
    {
        count = Set_len(Send_str_tmp, Send_str, hex_len, count);            //加入hex长度信息
        send_len += Send_Date(Send_str,(Send_str[0] & 0x0f) + 1);      //发送数据
        if(send_len < 0) 
            return -1;
        if(count == 0)
            break;
        /*     
        i = 0;
        while(1)
        {
            printf("%02x ",Send_str[i]);
            i++;
            if(i > (Send_str[0] & 0x0f))
                break;
        }
        printf(" count: %d\n", count);
        */
    }
    return send_len;
}

/*                      回传数据格式                       
                        0           1-14      15
回传数据小于16字节：
                    (0xc0 & len)    date    state
回传数据大于16字节：     
                        0x8f             date           起始
                        0X0f             date           中间
                    (0x40 & len)    date    state       结束
*/

//功能：从接收数据首字节获取hex数据的大小和状态，并清除该字节
//入口参数：str：hex流数据，len：保存hex数据的大小
//返回值：hex数据状态
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

static int error_num;
void Error_Detail(void)
{
    switch(error_num)
    {
        case 0:
            printf("指令执行成功\n");
        break;
        case 1:
            printf("指令执行失败\n");
        break;
        case 2:
            printf("指令不支持\n");
        break;
        case 3:
            printf("指令不支持\n");
        break;
        case 4:
            printf("读取数据失败\n");
        break;
        case 5:
            printf("用于保存数据的数组过小\n");
        break;
    }
}

//功能：获取接收的hex数据中的执行状态，并清除状态字节
//入口参数：Recv_str：hex数据，len：hex数据大小
//返回值：-1：指令执行失败， -2：指令不支持， 0：指令执行成功
int Get_Cmd_Sta(unsigned char *Recv_str, int len)
{
    if(Recv_str[0] == 0x6a && len == 1)
    {
        Recv_str[0] = '\0';
        error_num = 1;
        return -1;
    }
    else if(Recv_str[0] == 0x6d && len == 1)
    {
        Recv_str[0] = '\0';
        error_num = 2;
        return -2;
    }
    else if(Recv_str[len - 1] == 0x90)
    {
        Recv_str[len - 1] = '\0';
        error_num = 0;
        return 0;
    }
    return -2;
}

//功能：hex数据流拼接复制
//入口参数：str_o：原hex数据，len：原hex数据大小，str_d：拼接复制后的数据，count：拼接位置（0 就是复制）
//返回值：下一次拼接的位置
int Strcpy_Hex(unsigned char *str_o, int len, unsigned char *str_d, int count)
{
    int i;

    for(i = 0; i < len; i++)
        str_d[i + count] = str_o[i];
    
    return i + count;
}

//功能：接收hex数据
//入口参数：str：保存接收的hex流数据，len：用于接收数据的字符数组大小
//返回值：成功：收到有效数据的字节数， -1：接收失败， -2：str大小小于接收的数据的大小, -3：指令执行出错
int Recv_Value(unsigned char *str, int len)
{
    unsigned char Recv_str[16];
    unsigned char str_temp[128];
    unsigned char sta, recv_sta;;
    int recv_len = 0;
    int Value_len;
    int count = 0;

    while(1)
    {
        recv_sta = Recv_Date(Recv_str,16);  //读取16字节数据，回传是以16字节为一组
        if(recv_sta < 0) 
        {
            error_num = 4;
            return -1;
        }
        sta = Get_len(Recv_str, &Value_len);//获取回传hex数据大小状态字
        recv_len += Value_len;
        if(sta == 8 || sta == 12)           //0x8f或0xcx 的首字节，表示收到的第一组数据
            count = Strcpy_Hex(Recv_str,Value_len,str_temp,count);
        else if(sta == 4 || sta ==0)        //0x4x或0x0f 的首字节，表示收到的最后一组或中间组的数据
            count = Strcpy_Hex(Recv_str,Value_len,str_temp,count);
        if(sta == 12 || sta == 4)           //0xc0表示只有一组，0x4x表示最后一组，因此收到就停止接收
            break;
    }

    
    if(Get_Cmd_Sta(str_temp, recv_len) < 0)
        return -3;
    else if(recv_len - 1 > len)
    {
        error_num = 5;
        return -2;
    }         
    else
        Strcpy_Hex(str_temp,recv_len - 1,str,0);
    
    error_num = 0;
    return recv_len - 1;
}

//功能：获取随机数
//入口参数：保存随机数的字符数组（16）
//返回值：成功：随机数hex大小， -1：发送指令失败， -2：接收数据失败
int Get_Radom(unsigned char *str)
{
    int length;
    int ret;
    
    ret = Send_Cmd("01");
    
    if(ret < 0)
        return -1;

    length = Recv_Value(str,16);
    if(length < 0)
        return -2;

    return length;
}

//功能：验证PIN
//入口参数：PIN
//返回值：成功：0， -1：发送指令失败， -2：接收数据失败
int Verfiy_PIN(unsigned char *pin)
{
    int ret;
    unsigned char Recv_str[16];
    unsigned char Send_Str[30] = "27";

    strcat(Send_Str,pin);
    
    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 
    
    return 0;
}

//功能：生成公私钥对,48字节公钥，24字节私钥
//入口参数：Public_key：公钥保存字符数组，Private_key：私钥保存字符数组
//返回值：成功：收到的hex数据大小， -1：发送指令失败， -2：接收数据失败
int Generate_Key_Pair(unsigned char *Public_key, unsigned char *Private_key)
{
    int length;
    int ret,i;
    unsigned char Recv_str[72];
    
    ret = Send_Cmd("28");
    
    if(ret < 0)
        return -1;

    length = Recv_Value(Recv_str,72);
    if(length < 0)
        return -2;
    
    for(i = 0; i < 72; i++)
    {
        if(i < 48)
            Public_key[i] = Recv_str[i];
        else
            Private_key[i-48] =  Recv_str[i];
    }

    return length;
}

//功能：生成证书
//入口参数：24字节私钥
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Generate_Certificate(unsigned char *Private_key)
{
    int ret;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "34";

    strcat(Send_Str,Private_key);

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}

//功能：读取证书
//入口参数：Certificate：保存证书的字符数组（112）
//返回值：成功：收到的hex数据大小， -1：发送指令失败， -2：接收数据失败
int Get_Certificate(unsigned char *Certificate)
{
    int length;
    int ret;
    
    ret = Send_Cmd("35");
    
    if(ret < 0)
        return -1;

    length = Recv_Value(Certificate,112);
    if(length < 0)
        return -2;

    return length;
}

//功能：非对称验证
//入口参数：Radom：随机数，Signature_value：签名值（48）
//返回值：成功：收到的hex数据大小， -1：发送指令失败， -2：接收数据失败
int Asymmetric_Verification(unsigned char *Radom, unsigned char *Signature_value)
{
    int ret;
    int length;
    unsigned char Send_Str[60] = "36";

    strcat(Send_Str,Radom);

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    length = Recv_Value(Signature_value,48);
    if(length < 0)
        return -2; 

    return length;
}

//功能：设置对称认证密钥
//入口参数：，密钥（16）
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Set_Sym_Verification_Key(unsigned char *Key)
{
    int ret;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "42";

    strcat(Send_Str,Key);
    
    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}

//功能：对称认证
//入口参数：Radom：随机数，Ciphertext：密文（16）
//返回值：成功：收到的hex数据大小， -1：发送指令失败， -2：接收数据失败
int Symmetric_Verification(unsigned char *Radom, unsigned char *Ciphertext)
{
    int ret;
    int length;
    unsigned char Send_Str[60] = "37";

    strcat(Send_Str,Radom);

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    length = Recv_Value(Ciphertext,16);
    if(length < 0)
        return -2; 

    return length;
}

//功能：设置读写密钥
//入口参数：Key：密钥，RW：读写
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Set_RW_Key(unsigned char *Key, unsigned char RW)
{
    int ret;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "43";

    if(RW == 0)
        strcat(Send_Str, "00");
    else if(RW == 1)
        strcat(Send_Str, "01");
    else
        return -4;
    
    strcat(Send_Str,Key);
    
    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}

//功能：配置密文读写空间(一共4K空间，以页为单位，每页128字节，如0x08,密文空间8*128 = 1k， 明文空间4-1 = 3k)
//入口参数：密文空间页数
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Set_Ciphertext_Space(unsigned char *Page_Num)
{
    int ret;
    int length;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "47";

    strcat(Send_Str,Page_Num);

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}

//功能：加密写
//入口参数：address:写入地址（两字节hex形式）， date：写入数据（1字节hex形式）(已经加密的数据还是未加密？？？)
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Encryption_Write(unsigned char *address, unsigned char *date)
{
    int ret;
    int date_len;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "38";
    unsigned char temp;

    strcat(Send_Str,address);

    date_len = strlen(date) / 2;
    temp = (date_len >> 4) & 0x0f;
    if(temp < 10)              
        Send_Str[6] = temp + 48;
    else                        //hex为a-f
        Send_Str[6] = temp + 87;

    temp = date_len & 0x0f;
    if(temp < 10)              
        Send_Str[7] = temp + 48;
    else                        //hex为a-f
        Send_Str[7] = temp + 87;

    Send_Str[8] = '\0';
    strcat(Send_Str,date);
    
    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}

//功能：加密读
//入口参数：address:读取地址（两字节hex形式）， len：读取的大小（1字节hex形式）， date：读取的数据
//返回值：成功：收到的hex数据大小(大小不固定，加密后的数据)， -1：发送指令失败， -2：接收数据失败
int Encryption_Read(unsigned char *address, int len, unsigned char *date)
{
    int ret;
    int length;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "39";
    unsigned char temp;

    strcat(Send_Str,address);

    temp = (len >> 4) & 0x0f;
    if(temp < 10)              
        Send_Str[6] = temp + 48;
    else                        //hex为a-f
        Send_Str[6] = temp + 87;

    temp = len & 0x0f;
    if(temp < 10)              
        Send_Str[7] = temp + 48;
    else                        //hex为a-f
        Send_Str[7] = temp + 87;

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    length = Recv_Value(date,60);
    if(length < 0)
        return -2; 

    return length;
}

//功能：明文读
//入口参数：address:读取地址（两字节hex形式）， len：读取的大小（1字节hex形式）， date：读取的数据
//返回值：成功：收到的hex数据大小， -1：发送指令失败， -2：接收数据失败
int Definitely_Read(unsigned char *address, int len, unsigned char *date)
{
    int ret;
    int length;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "41";
    unsigned char temp;

    strcat(Send_Str,address);

    temp = (len >> 4) & 0x0f;
    if(temp < 10)              
        Send_Str[6] = temp + 48;
    else                        //hex为a-f
        Send_Str[6] = temp + 87;

    temp = len & 0x0f;
    if(temp < 10)              
        Send_Str[7] = temp + 48;
    else                        //hex为a-f
        Send_Str[7] = temp + 87;

    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    length = Recv_Value(date,60);
    if(length < 0)
        return -2; 

    return length;
}

//功能：明文写
//入口参数：address:写入地址（两字节hex形式）， date：写入数据（1字节hex形式）
//返回值：0：成功， -1：发送指令失败， -2：接收数据失败
int Definitely_Write(unsigned char *address, unsigned char *date)
{
    int ret;
    int date_len;
    unsigned char Recv_str[16];
    unsigned char Send_Str[60] = "40";
    unsigned char temp;

    strcat(Send_Str,address);

    date_len = strlen(date) / 2;
    temp = (date_len >> 4) & 0x0f;
    if(temp < 10)              
        Send_Str[6] = temp + 48;
    else                        //hex为a-f
        Send_Str[6] = temp + 87;

    temp = date_len & 0x0f;
    if(temp < 10)              
        Send_Str[7] = temp + 48;
    else                        //hex为a-f
        Send_Str[7] = temp + 87;

    Send_Str[8] = '\0';
    strcat(Send_Str,date);
    
    ret = Send_Cmd(Send_Str);
    if(ret < 0) 
        return -1;

    ret = Recv_Value(Recv_str,16);
    if(ret < 0)
        return -2; 

    return 0;
}