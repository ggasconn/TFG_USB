/*
 * Full loaded driver, allow communication between all the devices
 * using the CONTROL endpoint. 
 * 
 * The OLED screen is the only device not supported. Please, refer
 * to the BasicInterrupt driver for this matter.
 *
 * Copyright (C) 2023 (Guillermo Gascón y Javier Rodriguez-Avello)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 * This driver is based on the sample driver found in the
 * Linux kernel sources (drivers/usb/usb-skeleton.c) and
 * the Blinkstick driver (https:/github.com/jcsaezal)
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

/* read() operation */
#define GET_MSG

/* Led ring size */
#define LED_SIZE 12

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

/* Enum to map all device features */
typedef enum {
	GET_LED_COLOR,
	GET_TEXT_MSG,
	SET_FULL_RING,
	SET_LED_RING,
	SET_BUZZER_FREQUENCY,
	SET_BLINK_LED
} device_ops;

#define MAX_LEN_MESSAGE 255 // Max size GET buffer

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

static ssize_t blink_write(struct file *file, const char *user_buffer,
			  size_t count, loff_t *ppos)
{
	struct usb_blink *dev;
	int retval = 0;
	int r,g,b,led = 0;
	int freq,stop = 0;
	char* strcfg = NULL;
	device_ops op = -1;

	dev = file->private_data;
	

	if ((strcfg=kmalloc(count + 1, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if (copy_from_user(strcfg, user_buffer, count)){
		retval=-EFAULT;
		goto out_error;
	}
	strcfg[count]='\0';

	if (sscanf(strcfg,"setFullColor %d:%d:%d", &r, &g, &b) == 3 
			&& (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255)) {		
		op = SET_FULL_RING;
		printk(KERN_INFO ">> Full led %d %d %d", r, g, b);
		goto ok_path;
	} else if (sscanf(strcfg,"setLedColor %d:%d:%d:%d", &led, &r, &g, &b) == 4 
			&& (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255) && (led >= 0 && led <= LED_SIZE)) {
		op = SET_LED_RING;
		printk(KERN_INFO ">> Led %d %d %d %d", led, r, g, b);
		goto ok_path;
	} else if (sscanf(strcfg,"buzzer %d %d", &freq, &stop) == 2) {
		op = SET_BUZZER_FREQUENCY;
		printk(KERN_INFO ">> Buzzer %d %d", freq, stop);
		goto ok_path;
	} else if (sscanf(strcfg,"blinkLed %d", &stop) == 1) {
		op = SET_BLINK_LED;
		printk(KERN_INFO ">> LED %d", stop);
		goto ok_path;
	}

ok_path:
	kfree(strcfg);
	(*ppos)+=count;
	return count;

out_error:
	if (strcfg)
		kfree(strcfg);
	return retval;	
}

static ssize_t blink_read(struct file *file, char *user_buffer,
			  size_t count, loff_t *ppos)
{
	struct usb_blink *dev;
	int retval = 0;
	unsigned char* message;	
	int nr_bytes = 0;
	unsigned int wValue = 0;
	int msgSize = 0;
	device_ops op = -1;

	dev = file->private_data;
	
	if (*ppos>0)
		return 0;

	message = kmalloc(MAX_LEN_MESSAGE, GFP_DMA);

	/* zero fill*/
	memset(message, 0, MAX_LEN_MESSAGE);

	#ifdef GET_LED
		wValue = 0x1;
		msgSize = 4;
		op = GET_LED_COLOR;
	#elif defined(GET_MSG)
		wValue = 0x2;
		msgSize = 33;
		op = GET_TEXT_MSG;
	#endif

	retval=usb_control_msg_recv(dev->udev,	
			 0, 
			 USB_REQ_CLEAR_FEATURE, 
			//USB_DIR_IN | USB_TYPE_STANDARD	| USB_RECIP_DEVICE,
			 USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
			 wValue,	/* wValue = Descriptor index */
			 0x0002, 	/* wIndex = Endpoint # */
			 message,	/* Pointer to the message */ 
			 msgSize, /* message's size in bytes */
			 0, /* Dale lo necesario*/
			 GFP_DMA);

	if (retval)
		goto out_error;

	if (op == GET_LED_COLOR) {
		sprintf(message, "R: %d G: %d B: %d \n", message[1], message[2], message[3]);
	} else if (op == GET_TEXT_MSG) {
		message[33]='\n';
		message[34]='\0';
	}

	nr_bytes=strlen(message + 1); // Remove reportID

	if (copy_to_user(user_buffer, message + 1, strlen(message + 1))){
		retval=-EFAULT;
		goto out_error;
	}

	kfree(message);
	(*ppos)+=nr_bytes;

	return nr_bytes;

out_error:
	if (message)
		kfree(message);

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
	.read =		blink_read,			/* read() operation on the file */
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
	.name =		"pwnedDevice%d",  /* Pattern used to create device files */	
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
		 "PwnedDevice now available via pwnedDevice%d",
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

	dev_info(&interface->dev, "PwnedDevice device #%d has been disconnected", minor);
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
	.name =		"pwnedDevice",
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