#include "Panel.h"
#include "ICP209_cmd.h"

const char Theme[20] = "ICP209";
const char   END[20] = "******";
const char Item1[20] = "输入命令  ";
const char Item2[20] = "验证PIN";
const char Item3[20] = "获取随机数";
const char Item4[20] = "生成公私钥对";
const char Item5[20] = "生成证书  ";
const char Item6[20] = "读取证书";
const char Item7[20] = "非对称认证";
const char Item8[60] = "设置对称认证密钥";
const char Item9[20] = "对称认证  ";
const char Item10[20] = "设置读写密钥";
const char Item11[20] = "设置密文空间";
const char Item12[20] = "加密读";
const char Item13[20] = "加密写      ";
const char Item14[20] = "明文读";
const char Item15[20] = "明文写      ";
const char Item16[20] = "DEFAULT";

void Display_Panel(void)
{
    printf("/*********************%s**********************/\n",Theme);
    printf("01.%s          ", Item1);
    printf("02.%s\n", Item2);
    printf("03.%s          ", Item3);
    printf("04.%s\n", Item4);
    printf("05.%s          ", Item5);
    printf("06.%s\n", Item6);
    printf("07.%s          ", Item7);
    printf("08.%s\n", Item8);
    printf("09.%s          ", Item9);
    printf("10.%s\n", Item10);
    printf("11.%s        ", Item11);
    printf("12.%s\n", Item12);
    printf("13.%s        ", Item13);
    printf("14.%s\n", Item14);
    printf("15.%s        ", Item15);
    printf("16.%s\n", Item16);
    printf("00.Exit\n");
    printf("/**********************%s**********************/\n",END);
}

int Examine_Cmd(unsigned char *str)
{
    int len = strlen(str);
    int i = 0;

    for(i = 0; i < len; i++)
    {
        if((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z'))
            return 0;
        else
            return -1;
    }
} 

int Command_Mode(void)
{
    int ret;
    int length; 
    int examine_sta = 0;
    int str_len;
    int cmd_sta;	

    unsigned char Send_str[33];
    unsigned char Recv_str[128];
    unsigned char Recv_str_tmp[128];
    system("clear"); 
    while(1)
    {
        printf("输入指令(<exit>退出)：");
        scanf("%s", Send_str);

        if(strcmp(Send_str, "exit") == 0)
            break;

        examine_sta = Examine_Cmd(Send_str);
        if(examine_sta < 0)
        {
            printf("指令有误！\n");
            continue;
        }

        length = Send_Cmd(Send_str);
        
        if(length < 0) 
            printf("*** Send_Cmd failed! \n");    		
        else
            printf("Send_Cmd(%d bytes): %s\n", length, Send_str);

        length = Recv_Value(Recv_str_tmp,128);
        HextoStr(Recv_str_tmp, length, Recv_str);
        if(length < 0)
        {
            Error_Detail();
            printf("*** Recv_Value failed! \n");
        } 
        else
            printf("Recv_str(%d bytes): %s\n", length, Recv_str);
        
    } 
}