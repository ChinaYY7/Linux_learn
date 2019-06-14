#include "libusb.h"
#include "usb_hub.h"
#include "Deal_Error.h"
#include "Panel.h"
extern uint8_t Connect_Sta;

int main(void)
{
    int ret; 	

    libusb_context *ctx = NULL; 

    //ctrl+c 信号处理
    Signal_Deal_Ctc();
    
    //初始化用户设备信息
    struct userDevice user_device;	
    Init_Struct_userDevice(&user_device);

    //初始化USB
    ret = libusb_init(NULL);                       
    if(ret < 0)
        System_Error_Exit("*** initial USB lib failed!");

    //开启热插拔
    Register_Hotplug(ctx,user_device.idVendor, user_device.idProduct,&user_device);

    //检测特定设备是否连接
    printf("Wait connecting......\n");
    ret = Find_device(&user_device);
    if(ret < 0) 
    {			
        if(ret == -1)
            System_Error("*** Find device failed!");
        else
            printf("*** Not Find Device VID:0x%4x  PID:0x%4x\n",user_device.idVendor, user_device.idProduct);
    }
    else if(ret == 0)
    {
        Connect_Sta = 1;
        Display_Device_Info(&user_device);
    }

    uint8_t Display_sta = 1;
    int cmd;
    while(1)
    {
        if(user_device.online == False)
        {
            Display_sta = 1;
        }
        if(user_device.online == True)
        {
            if(Display_sta)
            {
                Display_Panel();
                Display_sta = 0;
            }
            printf("Input cmd:");
            scanf("%d", &cmd);
            if(cmd == 0)
                break;
            switch(cmd)
            {
                case 1:
                {
                    Command_Mode(user_device);
                    Display_sta = 1;
                    break;
                }
                case 2:
                    break;
                default:
                    printf("cmd is wrong\n");
            }
        }
    }
    
    libusb_close(user_device.usb_handle);
    libusb_release_interface(user_device.usb_handle,user_device.bInterfaceNumber); 	
    
    //libusb_exit(ctx);
    printf("Exit !!!\n"); 
}
