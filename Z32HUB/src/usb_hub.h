#ifndef _USB_HUB_H
#define _USB_HUB_H
#include "libusb.h"
#include "apue.h"

#define USB_VENDOR_ID 0x19fd
#define USB_PRODUCT_ID 0x3232

struct userDevice{
    uint16_t idVendor;              //VID
	uint16_t idProduct; 	        //PID
    
    
    uint8_t  bInterfaceClass;       //接口类型
    uint8_t  bInterfaceSubClass;    //接口子类型

    uint8_t  bmAttributes;          //传输模式

    u_int8_t bInEndpointAddress; 	//输入端口
    u_int8_t bOutEndpointAddress;   //输出端口
	
    libusb_device *dev;	            //设备标识
    struct libusb_device_descriptor dev_desc; //设备描述符

    libusb_device_handle* usb_handle; //设备句柄

    uint8_t bus;
    uint8_t address;

    Bool online;
    
    
    /* Number of this interface */	
    uint8_t  bInterfaceNumber;      //接口数
};
int Init_USB(void);
void Close_USB(void);
int Find_device(void);
int Open_device(void);
int Fill_UserDevice_Info(libusb_device *dev, struct libusb_device_descriptor dev_desc);
void Display_Device_Info(void);
void Init_Struct_userDevice(void);
int Register_Hotplug(void);
int Detect_Device_Connect_sta(void);
int Send_Date(unsigned char *str, int len);
int Recv_Date(unsigned char *str, int len);
#endif


