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
    unsigned char Recv_str[64];
    unsigned char Recv_str_tmp[64];
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
        /* 
        if(length < 0) 
            printf("*** Send_Cmd failed! \n");    		
        else
            printf("Send_Cmd(%d bytes): %s\n", length, Send_str);

        if(strcmp(Send_str,"01") == 0)
        {
            length = Recv_Value(Recv_str_tmp,16);
            strcpy(Recv_str,Recv_str_tmp);
            
            length+=Recv_Value(Recv_str_tmp,16);
            strcat(Recv_str, Recv_str_tmp);
        }
        else
            length = Recv_Value(Recv_str,16);

        cmd_sta = Get_Cmd_Sta(Recv_str);
        if(cmd_sta == -1)
            strcat(Recv_str,"  指令执行失败！");
        else if(cmd_sta == -2)
            strcat(Recv_str,"  指令不支持！");
        else
            strcat(Recv_str,"  指令执行成功！"); 
    
        if(length < 0) 
            printf("*** Recv_Value failed! \n");    			
        else
            printf("Recv_Value(%d bytes): %s\n", length, Recv_str);
            */
    } 
}