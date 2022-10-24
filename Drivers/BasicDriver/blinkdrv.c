/*
 * Simple driver for the Blinkstick Strip USB device (v1.0)
 *cd ..
 * Copyright (C) 2018 Juan Carlos Saez (jcsaezal@ucm.es)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the sample driver found in the
 * Linux kernel sources  (drivers/usb/usb-skeleton.c) 
 * 
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");

/* Get a minor range for your devices from the usb maintainer */
#define USB_BLINK_MINOR_BASE	0 

/* Structure to hold all of our device specific stuff */
struct usb_blink {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
};
#define to_blink_dev(d) container_of(d, struct usb_blink, kref)

static struct usb_driver blink_driver;

/* 
 * Free up the usb_blink structure and
 * decrement the usage count associated with the usb device 
 */
static void blink_delete(struct kref *kref)
{
	struct usb_blink *dev = to_blink_dev(kref);

	usb_put_dev(dev->udev);
	kfree(dev);
}

/* Called when a user program invokes the open() system call on the device */
static int blink_open(struct inode *inode, struct file *file)
{
	struct usb_blink *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	
	/* Obtain reference to USB interface from minor number */
	interface = usb_find_interface(&blink_driver, subminor);
	if (!interface) {
		pr_err("%s - error, can't find device for minor %d\n",
			__func__, subminor);
		return -ENODEV;
	}

	/* Obtain driver data associated with the USB interface */
	dev = usb_get_intfdata(interface);
	if (!dev)
		return -ENODEV;

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

	return retval;
}

/* Called when a user program invokes the close() system call on the device */
static int blink_release(struct inode *inode, struct file *file)
{
	struct usb_blink *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, blink_delete);
	return 0;
}


#define NR_LEDS 12
#define NR_BYTES_BLINK_MSG 5
#define REPORT_ID	0x4


static int parse_color_string(char* strconfig,unsigned char* colorspec){
	char* flag;
	char* cfg=strconfig;
	unsigned int colors;
	int nr_led;
	int error=0;
	int base_index=NR_BYTES_BLINK_MSG==5?2:1;

	/* Clean all */
	if (strcmp(strconfig,"\n")==0)
		return 0;

	while((flag = strsep(&cfg, ","))!=NULL)
        {
		if(sscanf(flag,"%i:0x%x", &nr_led, &colors)==2)
		{
			if ((nr_led<0 || nr_led>=NR_LEDS))
			{
				error=-EINVAL;
				printk(KERN_INFO "No such led: %i\n",nr_led);
				break;
			}
				
			colorspec[nr_led*NR_BYTES_BLINK_MSG+base_index]=((colors>>16) & 0xff);
 			colorspec[nr_led*NR_BYTES_BLINK_MSG+base_index+1]=((colors>>8) & 0xff);
 			colorspec[nr_led*NR_BYTES_BLINK_MSG+base_index+2]=(colors & 0xff);
		} else
		{
			error=-EINVAL;
			break;
		}

	}

	return error;

}


#define DS_A	0x80
#define DS_B	0x40
#define DS_C	0x20
#define DS_D	0x10
#define DS_E	0x08
#define DS_F	0x04
#define DS_G	0x02
#define DS_DP  0x01

#define LEDS_D0	(DS_A|DS_B|DS_C|DS_D|DS_E|DS_F)
#define LEDS_D1	(DS_B|DS_C)
#define LEDS_D2	(DS_A|DS_B|DS_G|DS_D|DS_E)
#define LEDS_D3	(DS_A|DS_B|DS_C|DS_D|DS_G)
#define LEDS_D4	(DS_B|DS_C|DS_F|DS_G)
#define LEDS_D5	(DS_A|DS_F|DS_G|DS_C|DS_D)
#define LEDS_D6	(DS_A|DS_C|DS_D|DS_E|DS_F|DS_G)
#define LEDS_D7	(DS_A|DS_B|DS_C)
#define LEDS_D8	(DS_A|DS_B|DS_C|DS_D|DS_E|DS_F|DS_G)
#define LEDS_D9	(DS_A|DS_B|DS_C|DS_F|DS_G)
#define LEDS_DA	(DS_A|DS_B|DS_C|DS_E|DS_F|DS_G)
#define LEDS_DB	(DS_A|DS_B|DS_C|DS_D|DS_E|DS_F|DS_G|DS_DP)
#define LEDS_DC	(DS_A|DS_D|DS_E|DS_F)
#define LEDS_DD	(DS_A|DS_B|DS_C|DS_D|DS_E|DS_F|DS_DP)
#define LEDS_DE	(DS_A|DS_D|DS_E|DS_F|DS_G)
#define LEDS_DF	(DS_A|DS_E|DS_F|DS_G)

const int display_code[]={LEDS_D0,LEDS_D1,LEDS_D2,LEDS_D3,LEDS_D4,
					LEDS_D5,LEDS_D6,LEDS_D7,LEDS_D8,LEDS_D9,
					LEDS_DA,LEDS_DB,LEDS_DC,LEDS_DD,LEDS_DE,LEDS_DF};


/* Script para probar el write 
$ while true; do for led in `seq 0 1 12`; do echo $led:0x100511  > /dev/usb/blinkstick0 ; sleep 0.25; done; done
*/

static ssize_t blink_write(struct file *file, const char *user_buffer,
			  size_t count, loff_t *ppos)
{

	struct usb_blink *dev;
	int retval = 0;
	int i=0;
	unsigned char* colors;	/* Matriz NR_LEDS x NR_BYTES_BLINK_MSG  */
	char* strcfg=NULL;

	dev = file->private_data;
	
	colors=kmalloc(NR_BYTES_BLINK_MSG*NR_LEDS,GFP_DMA);

	/* zero fill*/
	memset(colors,0,NR_LEDS*NR_BYTES_BLINK_MSG);

	if ((strcfg=kmalloc(count+1, GFP_KERNEL))==NULL)
		return -ENOMEM;

	if (copy_from_user(strcfg,user_buffer,count)){
		retval=-EFAULT;
		goto out_error;
	}
	strcfg[count]='\0';

	if (sscanf(strcfg,"7s %x",&i)==1 && (i>=0 && i<=15)){		
		colors[0]=0x5;
		colors[1]=display_code[i];


		retval=usb_control_msg(dev->udev,	
			 usb_sndctrlpipe(dev->udev,00), /* Specify endpoint #0 */
			 USB_REQ_SET_CONFIGURATION, 
			 USB_DIR_OUT| USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			 /* 0x200 | */ 5,	/* wValue */
			 0, 	/* wIndex=Endpoint # */
			 colors,	/* Pointer to the message */ 
			 2, /* message's size in bytes */
			 0);
		
		if (retval<0 && retval!=-EPIPE){
			printk(KERN_ALERT "Executed with retval=%d\n",retval);
			goto out_error;		
		}

		goto ok_path;
	}

	if ((retval=parse_color_string(strcfg,colors)))
		goto out_error;

	for (i=0;i<NR_LEDS;i++){
		/* Force reportID and LED number as it was not previously initialized */
		colors[i*NR_BYTES_BLINK_MSG]=REPORT_ID;
		if (NR_BYTES_BLINK_MSG==5)
			colors[i*NR_BYTES_BLINK_MSG+1]=i;

		retval=usb_control_msg(dev->udev,	
			 usb_sndctrlpipe(dev->udev,00), /* Specify endpoint #0 */
			 USB_REQ_SET_CONFIGURATION, 
			 USB_DIR_OUT| USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			 /* 0x200 | */ REPORT_ID,	/* wValue */
			 0, 	/* wIndex=Endpoint # */
			 &colors[i*NR_BYTES_BLINK_MSG],	/* Pointer to the message */ 
			 NR_BYTES_BLINK_MSG, /* message's size in bytes */
			 0);	


		if (retval<0 && retval!=-EPIPE){
			printk(KERN_ALERT "Executed with retval=%d\n",retval);
			goto out_error;		
		}
		if (NR_BYTES_BLINK_MSG==4)
			break;
	}

ok_path:
	kfree(strcfg);
	kfree(colors);
	(*ppos)+=count;
	return count;

out_error:
	if (strcfg)
		kfree(strcfg);
	kfree(colors);
	return retval;	
}


/*
 * Operations associated with the character device 
 * exposed by driver
 * 
 */
static const struct file_operations blink_fops = {
	.owner =	THIS_MODULE,
	.write =	blink_write,	 	/* write() operation on the file */
	.open =		blink_open,			/* open() operation on the file */
	.release =	blink_release, 		/* close() operation on the file */
};

/* 
 * Return permissions and pattern enabling udev 
 * to create device file names under /dev
 * 
 * For each blinkstick connected device a character device file
 * named /dev/usb/blinkstick<N> will be created automatically  
 */
char* set_device_permissions(struct device *dev, umode_t *mode) 
{
	if (mode)
		(*mode)=0666; /* RW permissions */
 	return kasprintf(GFP_KERNEL, "usb/%s", dev_name(dev)); /* Return formatted string */
}


/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver blink_class = {
	.name =		"blinkstick%d",  /* Pattern used to create device files */	
	.devnode=	set_device_permissions,	
	.fops =		&blink_fops,
	.minor_base =	USB_BLINK_MINOR_BASE,
};

/*
 * Invoked when the USB core detects a new
 * blinkstick device connected to the system.
 */
static int blink_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_blink *dev;
	int retval = -ENOMEM;

	/*
 	 * Allocate memory for a usb_blink structure.
	 * This structure represents the device state.
	 * The driver assigns a separate structure to each blinkstick device
 	 *
	 */
	dev = kmalloc(sizeof(struct usb_blink), GFP_KERNEL);

	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}

	/* Initialize the various fields in the usb_blink structure */
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &blink_class);
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */	
	dev_info(&interface->dev,
		 "Blinkstick device now attached to blinkstick-%d",
		 interface->minor);
	return 0;

error:
	if (dev)
		/* this frees up allocated memory */
		kref_put(&dev->kref, blink_delete);
	return retval;
}

/*
 * Invoked when a blinkstick device is 
 * disconnected from the system.
 */
static void blink_disconnect(struct usb_interface *interface)
{
	struct usb_blink *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &blink_class);

	/* prevent more I/O from starting */
	dev->interface = NULL;

	/* decrement our usage count */
	kref_put(&dev->kref, blink_delete);

	dev_info(&interface->dev, "Blinkstick device #%d has been disconnected", minor);
}

/* Define these values to match your devices */
#define BLINKSTICK_VENDOR_ID	0X20A0
#define BLINKSTICK_PRODUCT_ID	0X41E5

/* table of devices that work with this driver */
static const struct usb_device_id blink_table[] = {
	{ USB_DEVICE(BLINKSTICK_VENDOR_ID,  BLINKSTICK_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, blink_table);

static struct usb_driver blink_driver = {
	.name =		"blinkstick",
	.probe =	blink_probe,
	.disconnect =	blink_disconnect,
	.id_table =	blink_table,
};

/* Module initialization */
int blinkdrv_module_init(void)
{
   return usb_register(&blink_driver);
}

/* Module cleanup function */
void blinkdrv_module_cleanup(void)
{
  usb_deregister(&blink_driver);
}

module_init(blinkdrv_module_init);
module_exit(blinkdrv_module_cleanup);

