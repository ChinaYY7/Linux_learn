#include "usb_hub.h"


int Find_device(struct libusb_device_descriptor *dev_desc,struct userDevice *user_device)
{	
    int ret = -2;	
    ssize_t cnt;	
    int i = 0; 	
    
    libusb_device **devs;	
    libusb_device *dev; 	
    
    cnt = libusb_get_device_list(NULL, &devs); //check the device number	
    if (cnt < 0)		
        return (int) cnt; 	
        
    while (1) 
    {		
        dev = devs[i++];
        if(dev == NULL)
            return -2;
        ret = libusb_get_device_descriptor(dev,dev_desc);
        if(ret < 0) 
            return -1;		
        if(dev_desc->idProduct==user_device->idProduct &&dev_desc->idVendor==user_device->idVendor)		
        {			
            user_device->dev = dev;			
            user_device->devs = devs;			
            return 0;	
        }	
    }
}

int match_with_endpoint(const struct libusb_interface_descriptor * interface, struct userDevice *user_device)
{	
    int i;	
    int ret=0;
    printf("bNumEndpoints:%d\n",interface->bNumEndpoints);	
    for(i=0;i<interface->bNumEndpoints;i++)	
    {		
        printf("endpoint[%d].bmAttributes = %x\n",i,interface->endpoint[i].bmAttributes);
        if((interface->endpoint[i].bmAttributes&0x03)==user_device->bmAttributes)   //transfer type :bulk ,control, interrupt		
        {				
            if(interface->endpoint[i].bEndpointAddress&0x80)					//out endpoint & in endpoint				
            {					
                ret|=1;					
                user_device->bInEndpointAddress = interface->endpoint[i].bEndpointAddress;
                printf("endpoint[%x]: is input\n", interface->endpoint[i].bEndpointAddress); 
            }				
            else				
            {					
                ret|=2;					
                user_device->bOutEndpointAddress = interface->endpoint[i].bEndpointAddress;
                printf("endpoint[%x] is output\n", interface->endpoint[i].bEndpointAddress); 				
            }		
        }											
    }	
    if(ret==3)	
    {		
        return 1;	
    }	
    else	
    {		
        return 0;	
    }
}

int get_device_endpoint(struct libusb_device_descriptor *dev_desc,struct userDevice *user_device)
{	
    /*3.get device endpoint that you need */	
    int ret = -2;	
    int i,j,k;	
    struct libusb_config_descriptor *conf_desc;	
    u_int8_t isFind = 0;
    printf("bNumConfigurations:%d\n",dev_desc->bNumConfigurations);	
    for (i=0; i< dev_desc->bNumConfigurations; i++)	
    {		
        if(user_device->dev != NULL)			
            ret = libusb_get_config_descriptor(user_device->dev,i,&conf_desc);		
        if(ret < 0) 
        {			
            printf("*** libusb_get_config_descriptor failed! \n");    			
            return -1;		
        }
        printf("bNumInterfaces:%d\n",conf_desc->bNumInterfaces);
        for (j=0; j< conf_desc->bNumInterfaces; j++)		
        {			
            printf("num_altsetting:%d\n",conf_desc->interface[j].num_altsetting);
            for (k=0; k < conf_desc->interface[j].num_altsetting; k++)			
            {				
                printf("interface[%d].altsetting[%d].bInterfaceClass = %d\n",j,k,conf_desc->interface[j].altsetting[k].bInterfaceClass);
                if(conf_desc->interface[j].altsetting[k].bInterfaceClass==user_device->bInterfaceClass)				
                {					
                    if(match_with_endpoint(&(conf_desc->interface[j].altsetting[k]), user_device))					
                    {						
                        user_device->bInterfaceNumber = conf_desc->interface[j].altsetting[k].bInterfaceNumber;
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





