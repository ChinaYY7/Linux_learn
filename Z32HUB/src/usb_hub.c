#include <pthread.h>
#include <signal.h>
#include "usb_hub.h"

struct userDevice user_device;
libusb_context *ctx = NULL; 

//初始化结构体
void Init_Struct_userDevice(void)
{
    user_device.online = False;
    user_device.idProduct = USB_PRODUCT_ID;	
    user_device.idVendor =  USB_VENDOR_ID ;	
    user_device.bInterfaceClass = LIBUSB_CLASS_HID ;	
    user_device.bInterfaceSubClass = LIBUSB_CLASS_HID ;	
    user_device.bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT ;	
    user_device.dev = NULL;
}

//显示设备信息
void Display_Device_Info(void)
{
    printf("/******************Device Info**********************/\n");
    printf("VID:0x%4x           PID:0x%4x\n",user_device.idVendor, user_device.idProduct);
    printf("BUS:%d                Address:%d\n",user_device.bus, user_device.address);
    printf("IN :0x%2x             Out:0x%02x\n",user_device.bInEndpointAddress, user_device.bOutEndpointAddress);
    printf("/**************************************************/\n");
}

int Init_USB(void)
{
    int ret;
    
     //初始化USB
    ret = libusb_init(&ctx);
    if(ret < 0)
    {
        printf("*** initial USB lib failed!");
        return -1;
    }
    
    //初始化用户设备信息
    Init_Struct_userDevice();

    return 0;
}

void Close_USB(void)
{
    //libusb_exit(ctx);
    libusb_close(user_device.usb_handle);
    libusb_release_interface(user_device.usb_handle,user_device.bInterfaceNumber);
}

//找到输入输出端口
//输入输出都找到返回1，否则返回0
int match_with_endpoint(const struct libusb_interface_descriptor *interface)
{	
    int i;	
    int ret=0;

    //printf("bNumEndpoints:%d\n",interface->bNumEndpoints);
    for(i=0;i<interface->bNumEndpoints;i++)	//遍历端点
    {
        //printf("endpoint[%d].bmAttributes = %x\n",i,interface->endpoint[i].bmAttributes);
	    if((interface->endpoint[i].bmAttributes&0x03)==user_device.bmAttributes)   //找到对应传输模式的端口transfer type :bulk ,control, interrupt		
        {				
            if(interface->endpoint[i].bEndpointAddress&0x80)					//输入端口				
            {					
                ret|=1;					
                user_device.bInEndpointAddress = interface->endpoint[i].bEndpointAddress;
                //printf("endpoint[%x]: is input\n", interface->endpoint[i].bEndpointAddress);
            }				
            else		                                                        //输出端口		
            {					
                ret|=2;					
                user_device.bOutEndpointAddress = interface->endpoint[i].bEndpointAddress;
                //printf("endpoint[%x] is output\n", interface->endpoint[i].bEndpointAddress);
            }		
        }											
    }	
    if(ret==3)		
        return 1;	
    else			
        return 0;	
}

//找到对应传输模式的端点
//成功返回0，失败返回-1；
int get_device_endpoint(struct libusb_device_descriptor *dev_desc)
{		
    int ret = -2;	
    int i,j,k;	
    struct libusb_config_descriptor *conf_desc;	
    u_int8_t isFind = 0;

    //printf("bNumConfigurations:%d\n",dev_desc->bNumConfigurations);
    for (i=0; i< dev_desc->bNumConfigurations; i++)	//遍历配置描述符
    {		
        if(user_device.dev != NULL)			
            ret = libusb_get_config_descriptor(user_device.dev,i,&conf_desc);		
        if(ret < 0) 
        {			
            printf("*** libusb_get_config_descriptor failed! \n");    			
            return -1;		
        }

        //printf("bNumInterfaces:%d\n",conf_desc->bNumInterfaces);
        for (j=0; j< conf_desc->bNumInterfaces; j++)	//遍历接口描述符	
        {
		    //printf("num_altsetting:%d\n",conf_desc->interface[j].num_altsetting);
            for (k=0; k < conf_desc->interface[j].num_altsetting; k++)			
            {
                //printf("interface[%d].altsetting[%d].bInterfaceClass = %d\n",j,k,conf_desc->interface[j].altsetting[k].bInterfaceClass);
                if(conf_desc->interface[j].altsetting[k].bInterfaceClass==user_device.bInterfaceClass)	 //找到对应接口模式的接口HID，print等			
                {					
                    if(match_with_endpoint(&(conf_desc->interface[j].altsetting[k])))					
                    {						
                        user_device.bInterfaceNumber = conf_desc->interface[j].altsetting[k].bInterfaceNumber;
                        libusb_free_config_descriptor(conf_desc);						
                        ret = 0;						
                        return ret;					
                    }				
                }			
            }		
        }	
    }	
    return -2;  //don't find user device
}


//打开设备，申请接口
int Open_device(void)
{
    int ret;
    user_device.usb_handle = libusb_open_device_with_vid_pid(NULL, user_device.idVendor, user_device.idProduct);

    if(user_device.usb_handle == NULL) 
    {		
        printf("*** Permission denied or Can not find the USB board (Maybe the USB driver has not been installed correctly), quit!\n");		
        return -1;	
    } 	
    
    //检测设备驱动是否被占用，若占用先卸载再重新安装
    if(libusb_kernel_driver_active(user_device.usb_handle, 0) == 1) 
    { 
        printf("kernel driver active\n");
        if(libusb_detach_kernel_driver(user_device.usb_handle, 0) == 0) //detach it
            printf("kernel driver detach\n");
    }

    //声明接口
    ret = libusb_claim_interface(user_device.usb_handle, 0); //声明接口，通常只有一个接口，默认为0
    if (ret < 0) 
    {
        printf("libusb_claim_interface failed:  %s\n", libusb_error_name(ret));
        return -2;
    }
    user_device.online = True;
    
    return 0;
}

//填充设备信息
int Fill_UserDevice_Info(libusb_device *dev, struct libusb_device_descriptor dev_desc)
{
    int ret;
    user_device.dev = dev;			
    user_device.dev_desc = dev_desc;
    user_device.bus = libusb_get_bus_number(dev);
    user_device.address = libusb_get_device_address(dev);
    ret = get_device_endpoint(&dev_desc);
    if(ret < 0)
    {
        return -1;
        user_device.online = False;
    }
        
    return 0;    
}

//找到设备，并找到对应的输入输出端点
//成功返回0，出错返回-1,未找到返回-2
int Find_device(void)
{	
    int ret = -2;	
    ssize_t cnt;	
    int i = 0; 	
    
    libusb_device **devs;	
    libusb_device *dev;

    struct libusb_device_descriptor dev_desc;
    
    cnt = libusb_get_device_list(ctx, &devs); //check the device number	
    if (cnt < 0)		
        return (int) cnt; 	
        
    while (1) 
    {		
        dev = devs[i++];
        if(dev == NULL)
        {
            libusb_free_device_list(devs, 1);
            return -2;
        }
        ret = libusb_get_device_descriptor(dev,&dev_desc);
        if(ret < 0) 
            return -1;		
        if(dev_desc.idProduct==user_device.idProduct && dev_desc.idVendor==user_device.idVendor)		
        {			
            libusb_free_device_list(devs, 1);
            Fill_UserDevice_Info(dev, dev_desc);
            Open_device();
            return 0;
        }
        else
            user_device.online = False;
    }
}

//USB热插拔监听线程
void *usb_monitor_thread(void *arg) 
{ 
    int ret;
    
    printf("usb monitor thread started.\n");
    while (1) 
    {
        ret = libusb_handle_events(ctx);
        if (ret < 0)
            printf("libusb_handle_events() failed: %s\n", libusb_error_name(ret));
    }  
}

//usb 热插拔回调函数
int LIBUSB_CALL usb_event_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data) 
{
    struct libusb_device_descriptor dev_desc;
    int ret;
    
    printf("\nUSB Hotplugin Event.\n");

    ret = libusb_get_device_descriptor(dev, &dev_desc);
    if (LIBUSB_SUCCESS != ret) 
        printf("error getting device descriptor.\n");
    
    //设备插入
    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) 
    {
        printf("USB device attached: %04x:%04x\n", dev_desc.idVendor, dev_desc.idProduct);
        
        //关闭之前设备断开后可能未能关闭的设备句柄
        if (user_device.usb_handle) 
        {
            libusb_close(user_device.usb_handle);
            user_device.usb_handle = NULL;
        }
        
        //重新打开设备获得新的设备句柄
        ret = Open_device();
        if (ret < 0)
        {
            printf ("error opening device.\n");
            return -1;
        }
        //填充信息 
        ret = Fill_UserDevice_Info(dev, dev_desc);
        if (ret < 0)
        {
            printf ("error fill info.\n");
            return -1;
        }
        Display_Device_Info();
    }        
    //设备拔出
    else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) 
    {
        printf("usb device removed: %04x:%04x\n", dev_desc.idVendor, dev_desc.idProduct);
        if (user_device.usb_handle) 
        {           
            libusb_close(user_device.usb_handle);
            user_device.online = False;
            user_device.usb_handle = NULL;
        }
    }

    return 0;
}

//启用热插拔功能
//返回-2，不支持，-1，启用失败
int Register_Hotplug(void)
{
    int ret;
    //检查是否支持热插拔
    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) 
    {
        printf("hotplug capabilites are not supported on this platform.\n");
        libusb_exit(ctx);
        return -2;
    }

    //注册热插拔事件
    ret = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_NO_FLAGS, user_device.idVendor, user_device.idProduct, LIBUSB_HOTPLUG_MATCH_ANY, usb_event_callback, NULL, NULL);
    if (LIBUSB_SUCCESS != ret) 
    {
        printf("error registering callback: %s\n", libusb_error_name(ret));
        libusb_exit(ctx);
        return -1;
    }

    //创建热插拔监听线程
    pthread_t usb_monitor_thread_id;
    ret = pthread_create(&usb_monitor_thread_id, 0, usb_monitor_thread, 0);
    if(ret != 0 )
    {
        perror("usb_monitor_thread creation faild\n");
        return -3;
    }

    return 0;
}

//检测设备是否处于连接状态
int Detect_Device_Connect_sta(void)
{
    if(user_device.online == False)
        return 0;
    else
        return 1;
}

//发送数据
//发送成功返回实际发送的字节数，失败返回-1
int Send_Date(unsigned char *str, int len)
{
    int ret;
    int length;

    ret = libusb_interrupt_transfer(user_device.usb_handle,user_device.bOutEndpointAddress,str,len,&length,1000);
    if(ret < 0) 
    {		
        printf("   %s\n", libusb_error_name(ret));
        printf("*** interrupt_transfer_out failed! \n");    		
        return -1;	
    }

    return length;
}

//接收数据
//接收成功返回实际接收的字节数，失败返回-1
int Recv_Date(unsigned char *str, int len)
{
    int ret;
    int length;
    
    ret = libusb_interrupt_transfer(user_device.usb_handle,user_device.bInEndpointAddress,str,len,&length,5000);
    if(ret < 0) 
    {		
        printf("   %s\n", libusb_error_name(ret));
        printf("*** interrupt_transfer_in failed! \n");    		
        return -1;	
    }

    return length;
}

