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
	
    libusb_device *dev;	            //设备标识
    struct libusb_device_descriptor dev_desc; //设备描述符

    libusb_device_handle* usb_handle; //设备句柄

    uint8_t bus;
    uint8_t address;

    Bool online;
    
    
    /* Number of this interface */	
    uint8_t  bInterfaceNumber;      //接口数
};

int Find_device(struct userDevice *user_device);
int match_with_endpoint(const struct libusb_interface_descriptor * interface, struct userDevice *user_device);
int get_device_endpoint(struct libusb_device_descriptor *dev_desc,struct userDevice *user_device);
int LIBUSB_CALL usb_event_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
int Open_device(struct userDevice *user_device);
int Fill_UserDevice_Info(struct userDevice *user_device, libusb_device *dev, struct libusb_device_descriptor dev_desc);
void Display_Device_Info(struct userDevice *user_device);
void Init_Struct_userDevice(struct userDevice *user_device);
int Register_Hotplug(libusb_context *ctx, int vendor_id, int product_id, struct userDevice *user_device);
void Signal_Deal_Ctc(void);
void StrToByte_stream(unsigned char *str_o, unsigned char *str_d);
void Byte_streamToStr(unsigned char *str_o, unsigned char *str_d);
int Examine_Cmd(unsigned char *str);
#endif


