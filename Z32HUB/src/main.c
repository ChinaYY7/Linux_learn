#include "libusb.h"
#include "usb_hub.h"
#include "Deal_Error.h"

void StrToByte_stream(unsigned char *str_o, unsigned char *str_d)
{
    unsigned char len = strlen(str_o);
    int i = 0, j = 1;
    int high_sta = 1;

    str_d[0] = 0xc0 | (len / 2);
    printf("%x ", str_d[0]);

    while(i < 32)
    {
        if(high_sta)
        {
            str_d[j] = ((str_o[i]-48) << 4) & 0xf0;
            //printf("str_d[%d]:%x ",j, str_d[j]);
            i++;
            high_sta = 0;
        }
        else
        {
            str_d[j] |= ((str_o[i]-48) & 0x0f);
            printf("%02x ", str_d[j]);
            j++;
            i++;
            high_sta = 1;
        }
    }
    printf("\n");
}

int main(void)
{
    int ret; 	
    int length; 	
    unsigned char Cmd_str_tmp[32] = "4000010188";
    unsigned char Cmd_str[16];
    unsigned char return_v[20];
    libusb_device_handle* g_usb_handle; 

    StrToByte_stream(Cmd_str_tmp, Cmd_str);
    
    struct userDevice user_device;	
    struct libusb_device_descriptor dev_desc; 	
    
    user_device.idProduct = USB_PRODUCT_ID;	
    user_device.idVendor =  USB_VENDOR_ID ;	
    user_device.bInterfaceClass = LIBUSB_CLASS_HID ;	
    user_device.bInterfaceSubClass = LIBUSB_CLASS_HID ;	
    user_device.bmAttributes = LIBUSB_TRANSFER_TYPE_INTERRUPT ;	
    user_device.dev = NULL;

    uint8_t path[8];
    int j,r;
    
    ret = libusb_init(NULL);                        //初始化USB
    if(ret < 0)
        System_Error_Exit("*** initial USB lib failed!");

    printf("Wait connecting......\n");
    //检测特定设备是否连接
    
    while(1)
    {
        ret = Find_device(&dev_desc,&user_device);
        if(ret < 0) 
        {			
            if(ret == -1)
                System_Error("*** get_device_descriptor failed!");
        }
        else if(ret == 0)
        {
            printf("Find device:  %04x-%04x (bus %d, device %d)",
			dev_desc.idVendor, dev_desc.idProduct,
			libusb_get_bus_number(user_device.dev), libusb_get_device_address(user_device.dev));
            r = libusb_get_port_numbers(user_device.dev, path, sizeof(path));
            if (r > 0) 
            {
                printf(" path: %d", path[0]);
                for (j = 1; j < r; j++)
                    printf(".%d", path[j]);
            }
            printf("\n");
            break;
        }
    }
    //while(1);
    
    ret = get_device_endpoint(&dev_desc,&user_device);	
    if(ret < 0) 
    {			
        printf("*** get_device_endpoint failed! ret:%d \n",ret);    			
        return -1;		
    } 		
    
    //open the usb device	
    g_usb_handle = libusb_open_device_with_vid_pid(NULL, user_device.idVendor, user_device.idProduct);	
    if(g_usb_handle == NULL) 
    {		
        printf("*** Permission denied or Can not find the USB board (Maybe the USB driver has not been installed correctly), quit!\n");		
        return -1;	
    } 	
    
    ret = libusb_claim_interface(g_usb_handle,user_device.bInterfaceNumber);	
    if(ret < 0) 
    {		
        printf("the Interface have been used\n");
        printf("detach the Interface\n");
        ret = libusb_detach_kernel_driver(g_usb_handle,user_device.bInterfaceNumber);		
        if(ret < 0) 
        {			
            printf("*** libusb_detach_kernel_driver failed! ret%d\n",ret);    			
            return -1;		
        }
        
        ret = libusb_claim_interface(g_usb_handle,user_device.bInterfaceNumber);		
        if(ret < 0)		
        {			
            printf("*** libusb_claim_interface failed! ret%d\n",ret);    			
            return -1;		
        } 	
    }
    ret = libusb_interrupt_transfer(g_usb_handle,0x02,Cmd_str,16,&length,0);		
    if(ret < 0) 
    {		
        printf("   %s\n", libusb_strerror((enum libusb_error)ret));
        printf("*** interrupt_transfer_out failed! \n");    		
        return -1;	
    }
    else
    {
        printf("send %d bytes\n", length);
    }
    ret = libusb_interrupt_transfer(g_usb_handle,0x81,return_v,16,&length,0);
    if(ret < 0) 
    {		
        printf("   %s\n", libusb_strerror((enum libusb_error)ret));
        printf("*** interrupt_transfer_int failed! \n");    		
        return -1;	
    }
    else
    {
        //printf("recv %d bytes: %s\n", length, return_v);
    }
    printf("finished!!!!\n");
    
    
    libusb_close(g_usb_handle); 	
    libusb_release_interface(g_usb_handle,user_device.bInterfaceNumber); 	
    libusb_free_device_list(user_device.devs, 1); 	
    libusb_exit(NULL);
}
