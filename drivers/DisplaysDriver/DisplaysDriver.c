/*
 * This driver communicates with the device when DISPLAYS template
 * is flashed.
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

/* Get a minor range for your devices from the usb maintainer */
#define USB_BLINK_MINOR_BASE	0 

/* Structure to hold all of our device specific stuff */
struct usb_pwnedDevice {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
};
#define to_pwnedDevice_dev(d) container_of(d, struct usb_pwnedDevice, kref)

static struct usb_driver pwnedDevice_driver;

#define MAX_LEN_MESSAGE 255 // Max size GET buffer

/* 
 * Free up the usb_pwnedDevice structure and
 * decrement the usage count associated with the usb device 
 */
static void pwnedDevice_delete(struct kref *kref) {
	struct usb_pwnedDevice *dev = to_pwnedDevice_dev(kref);

	usb_put_dev(dev->udev);
	kfree(dev);
}

/* Called when a user program invokes the open() system call on the device */
static int pwnedDevice_open(struct inode *inode, struct file *file) {
	struct usb_pwnedDevice *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	
	/* Obtain reference to USB interface from minor number */
	interface = usb_find_interface(&pwnedDevice_driver, subminor);
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
static int pwnedDevice_release(struct inode *inode, struct file *file) {
	struct usb_pwnedDevice *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, pwnedDevice_delete);
	return 0;
}

static ssize_t pwnedDevice_write(struct file *file, const char *user_buffer,
			  				size_t count, loff_t *ppos) {
	struct usb_pwnedDevice *dev;
	int retval = 0;
	unsigned int digit = 0;
	char* strcfg = NULL;
	unsigned int wValue = 0;
	int dataSize = 0;
	char *buffer;

	dev = file->private_data;
	
	if ((strcfg=kmalloc(count + 1, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if (copy_from_user(strcfg, user_buffer, count)){
		retval=-EFAULT;
		goto out_error;
	}
	strcfg[count] = '\0';

	if ((buffer = kmalloc(MAX_LEN_MESSAGE, GFP_KERNEL)) == NULL) {
		retval = -ENOMEM;
		goto out_error;
	}

	if (sscanf(strcfg, "oled %s", buffer) != 0) {
		/* TODO: REVIEW! SSCANF DOESNT TAKE SPACES! %[^\n] */
		strcpy(buffer, strcfg);
		dataSize = count;
		wValue = 3;
	} else if (sscanf(strcfg,"7s %x", &digit) == 1) {
		buffer[0] = 4; // ReportID
		buffer[1] = digit;
		dataSize = 2;
		wValue = 4;
	}

	if (wValue != 0) {
		retval = usb_control_msg(dev->udev,	
								usb_sndctrlpipe(dev->udev, 00), /* Specify endpoint #0 */
								USB_REQ_SET_CONFIGURATION, 
								USB_DIR_OUT| USB_TYPE_CLASS | USB_RECIP_INTERFACE,
								wValue,	/* wValue */
								0, 	/* wIndex=Endpoint # */
								buffer,	/* Pointer to the message */ 
								dataSize, /* message's size in bytes */
								0);
	}
	
	if (retval < 0 && retval != -EPIPE){
		printk(KERN_ALERT "Executed with retval=%d\n",retval);
		goto out_error;		
	}

	goto ok_path;

ok_path:
	kfree(strcfg);
	kfree(buffer);
	(*ppos)+=count;
	return count;

out_error:
	if (strcfg)
		kfree(strcfg);
	if (buffer)
		kfree(buffer);
	return retval;	
}

static ssize_t pwnedDevice_read(struct file *file, char *user_buffer,
			  				size_t count, loff_t *ppos) {
	struct usb_pwnedDevice *dev;
	int retval = 0;
	unsigned char* message;	
	int nr_bytes = 0;
	unsigned int wValue = 0;
	int msgSize = 0;

	dev = file->private_data;
	
	if (*ppos>0)
		return 0;

	message = kmalloc(MAX_LEN_MESSAGE, GFP_DMA);

	/* zero fill*/
	memset(message, 0, MAX_LEN_MESSAGE);

	wValue = 0x1;
	msgSize = 33;

	retval = usb_control_msg_recv(dev->udev,	
									0, 
									USB_REQ_CLEAR_FEATURE, 
									USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_DEVICE,
									wValue,	/* wValue = Descriptor index */
									0x0, 	/* wIndex = Endpoint # */
									message,	/* Pointer to the message */ 
									msgSize, /* message's size in bytes */
									0, /* Dale lo necesario*/
									GFP_DMA);

	if (retval)
		goto out_error;

	message[33]='\n';
	message[34]='\0';
	nr_bytes = strlen(message + 1); // Remove reportID
	
	if (copy_to_user(user_buffer, message + 1, strlen(message + 1))){
		retval=-EFAULT;
		goto out_error;
	}

	kfree(message);
	(*ppos) += nr_bytes;

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
static const struct file_operations pwnedDevice_fops = {
	.owner =	THIS_MODULE,
	.write =	pwnedDevice_write,	 	/* write() operation on the file */
	.read =		pwnedDevice_read,			/* read() operation on the file */
	.open =		pwnedDevice_open,			/* open() operation on the file */
	.release =	pwnedDevice_release, 		/* close() operation on the file */
};

/* 
 * Return permissions and pattern enabling udev 
 * to create device file names under /dev
 * 
 * For each pwnedDevicestick connected device a character device file
 * named /dev/usb/pwnedDevicestick<N> will be created automatically  
 */
char* set_device_permissions(struct device *dev, umode_t *mode) {
	if (mode)
		(*mode)=0666; /* RW permissions */
 	return kasprintf(GFP_KERNEL, "usb/%s", dev_name(dev)); /* Return formatted string */
}


/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver pwnedDevice_class = {
	.name =		"pwnedDevice%d",  /* Pattern used to create device files */	
	.devnode=	set_device_permissions,	
	.fops =		&pwnedDevice_fops,
	.minor_base =	USB_BLINK_MINOR_BASE,
};

/*
 * Invoked when the USB core detects a new
 * pwnedDevicestick device connected to the system.
 */
static int pwnedDevice_probe(struct usb_interface *interface,
		      			const struct usb_device_id *id) {
	struct usb_pwnedDevice *dev;
	int retval = -ENOMEM;

	/*
 	 * Allocate memory for a usb_pwnedDevice structure.
	 * This structure represents the device state.
	 * The driver assigns a separate structure to each pwnedDevicestick device
 	 *
	 */
	dev = kmalloc(sizeof(struct usb_pwnedDevice), GFP_KERNEL);

	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}

	/* Initialize the various fields in the usb_pwnedDevice structure */
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &pwnedDevice_class);
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
		kref_put(&dev->kref, pwnedDevice_delete);
	return retval;
}

/*
 * Invoked when a pwnedDevicestick device is 
 * disconnected from the system.
 */
static void pwnedDevice_disconnect(struct usb_interface *interface) {
	struct usb_pwnedDevice *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &pwnedDevice_class);

	/* prevent more I/O from starting */
	dev->interface = NULL;

	/* decrement our usage count */
	kref_put(&dev->kref, pwnedDevice_delete);

	dev_info(&interface->dev, "PwnedDevice device #%d has been disconnected", minor);
}

/* Define these values to match your devices */
#define BLINKSTICK_VENDOR_ID	0X20A0
#define BLINKSTICK_PRODUCT_ID	0X41E5

/* table of devices that work with this driver */
static const struct usb_device_id pwnedDevice_table[] = {
	{ USB_DEVICE(BLINKSTICK_VENDOR_ID,  BLINKSTICK_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, pwnedDevice_table);

static struct usb_driver pwnedDevice_driver = {
	.name =		"pwnedDevice",
	.probe =	pwnedDevice_probe,
	.disconnect =	pwnedDevice_disconnect,
	.id_table =	pwnedDevice_table,
};

/* Module initialization */
int pwnedDevicedrv_module_init(void) {
   return usb_register(&pwnedDevice_driver);
}

/* Module cleanup function */
void pwnedDevicedrv_module_cleanup(void) {
  usb_deregister(&pwnedDevice_driver);
}

module_init(pwnedDevicedrv_module_init);
module_exit(pwnedDevicedrv_module_cleanup);