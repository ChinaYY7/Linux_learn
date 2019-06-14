#include "Panel.h"
const char Theme[20] = "ICP209";
const char   END[20] = "******";
const char Item1[20] = "输入命令";
const char Item2[20] = "DEFAULT";
const char Item3[20] = "DEFAULT";
const char Item4[20] = "DEFAULT";
const char Item5[20] = "DEFAULT";
const char Item6[20] = "DEFAULT";
const char Item7[20] = "DEFAULT";
const char Item8[20] = "DEFAULT";
const char Item9[20] = "DEFAULT";
const char Item10[20] = "DEFAULT";

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

int Command_Mode(struct userDevice user_device)
{
    int ret;
    int length; 
    int examine_sta = 0;	
    unsigned char Send_cmd[16] = "4000010188";
    unsigned char Send_cmd_temp[33];
    unsigned char Recv_date[16];
    unsigned char Recv_date_temp[33];
    system("clear"); 
    while(1)
    {
        printf("输入指令(<exit>退出)：");
        scanf("%s", Send_cmd_temp);

        if(strcmp(Send_cmd_temp, "exit") == 0)
            break;

        examine_sta = Examine_Cmd(Send_cmd_temp);
        if(examine_sta < 0)
        {
            printf("指令有误！\n");
            continue;
        }

        StrToByte_stream(Send_cmd_temp, Send_cmd);
        ret = libusb_interrupt_transfer(user_device.usb_handle,user_device.bOutEndpointAddress,Send_cmd,16,&length,1000);
        if(ret < 0) 
        {		
            printf("   %s\n", libusb_error_name(ret));
            printf("*** interrupt_transfer_out failed! \n");    		
            return -1;	
        }
        else
            printf("Send_cmd(%d bytes): %s\n", length, Send_cmd_temp);

        ret = libusb_interrupt_transfer(user_device.usb_handle,user_device.bInEndpointAddress,Recv_date,16,&length,5000);
        if(ret < 0) 
        {		
            printf("   %s\n", libusb_error_name(ret));
            printf("*** interrupt_transfer_int failed! \n");    		
            return -1;	
        }
        else
        {
            Byte_streamToStr(Recv_date,Recv_date_temp);
            printf("Recv_date(%d bytes): %s\n", length, Recv_date_temp);
        }
    } 
}