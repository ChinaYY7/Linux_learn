#include "usb_hub.h"
#include "Deal_Error.h"
#include "Panel.h"
#include "ICP209_cmd.h"

int main(void)
{
    int ret; 	

    //初始化USB
    Init_USB();

    //ctrl+c 信号处理
    Signal_Deal_Ctc();
    
    //开启热插拔
    Register_Hotplug();

    //检测特定设备是否连接
    printf("Wait connecting......\n");
    ret = Find_device();
    if(ret < 0) 
    {			
        if(ret == -1)
            System_Error("*** Find device failed!");
        else
            printf("*** Not Find Device\n");
    }
    else if(ret == 0)
        Display_Device_Info();
    
    uint8_t Display_sta = 1;
    int Verfiy_PIN_sta;
    int Get_Radom_Sta;
    unsigned char Radom_Str[64];
    unsigned char *PIN = "27e1c9aaa518389fd0";

    int cmd;

    while(1)
    {
        if(!Detect_Device_Connect_sta())
            Display_sta = 1;
        
        if(Detect_Device_Connect_sta)
        {
            if(Display_sta)
            {
                Display_Panel();
                Display_sta = 0;
            }
            printf("Input cmd(cmd:11 clear):");
            scanf("%d", &cmd);
            if(cmd == 0)
                break;
            switch(cmd)
            {
                case 1:
                {
                    Command_Mode();
                    Display_sta = 1;
                    break;
                }
                case 2:
                    Verfiy_PIN_sta = Verfiy_PIN(PIN);
                    if(Verfiy_PIN_sta < 0)
                        printf("PIN(%s):验证失败！\n", PIN);
                    else
                        printf("PIN(%s):验证成功！\n", PIN);
                    break;
                case 3:
                    Get_Radom_Sta = Get_Radom(Radom_Str);
                    if(Get_Radom_Sta < 0)
                        printf("获取随机数失败");
                    else
                        printf("随机数为(%ld bytes): %s\n", strlen(Radom_Str) / 2, Radom_Str);
                    break;
                case 11:
                    system("clear");
                    Display_sta = 1;
                    break;
                default:
                    printf("cmd is wrong\n");
            }
        }
    }
    
    Close_USB();
    printf("Exit !!!\n"); 
}
