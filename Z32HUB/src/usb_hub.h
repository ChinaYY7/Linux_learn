#ifndef _USB_HUB_H
#define _USB_HUB_H
#include "libusb.h"
#include "apue.h"

#define USB_VENDOR_ID 0x19fd
#define USB_PRODUCT_ID 0x3232

#define BULK_ENDPOINT_OUT 1
#define BULK_ENDPOINT_IN  2

struct userDevice{
    uint16_t idVendor;              //VID
	uint16_t idProduct; 	        //PID
    
    
    uint8_t  bInterfaceClass;       //接口类型
    uint8_t  bInterfaceSubClass;    //接口子类型

    uint8_t  bmAttributes;          //传输模式

    u_int8_t bInEndpointAddress; 	//输入端口
    u_int8_t bOutEndpointAddress;   //输出端口

    /*save parameter*/ 	
    libusb_device *dev;	
    libusb_device **devs; 
    
    /* Number of this interface */	
    uint8_t  bInterfaceNumber;      //接口数
};

int Find_device(struct libusb_device_descriptor *dev_desc,struct userDevice *user_device);
int match_with_endpoint(const struct libusb_interface_descriptor * interface, struct userDevice *user_device);
int get_device_endpoint(struct libusb_device_descriptor *dev_desc,struct userDevice *user_device);

#endif


