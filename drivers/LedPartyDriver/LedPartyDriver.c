/*
 * This driver communicates with the device when LED_PARTY template
 * is flashed.
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

/* Led ring size */
#define LED_SIZE 12

/* Get a minor range for your devices from the usb maintainer */
#define USB_BLINK_MINOR_BASE	0 

/* Structure to hold all of our device specific stuff */
struct usb_ledParty {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
};
#define to_ledParty_dev(d) container_of(d, struct usb_ledParty, kref)

static struct usb_driver ledParty_driver;

#define MAX_LEN_MESSAGE 255 // Max size GET buffer

/* 
 * Free up the usb_ledParty structure and
 * decrement the usage count associated with the usb device 
 */
static void ledParty_delete(struct kref *kref) {
	struct usb_ledParty *dev = to_ledParty_dev(kref);

	usb_put_dev(dev->udev);
	kfree(dev);
}

/* Called when a user program invokes the open() system call on the device */
static int ledParty_open(struct inode *inode, struct file *file) {
	struct usb_ledParty *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	
	/* Obtain reference to USB interface from minor number */
	interface = usb_find_interface(&ledParty_driver, subminor);
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
static int ledParty_release(struct inode *inode, struct file *file) {
	struct usb_ledParty *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, ledParty_delete);
	return 0;
}

static ssize_t ledParty_write(struct file *file, const char *user_buffer,
			  				size_t count, loff_t *ppos) {
	struct usb_ledParty *dev;
	int retval = 0;
	int r,g,b,led = 0;
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

	if (sscanf(strcfg,"setFullColor %d:%d:%d", &r, &g, &b) == 3 
			&& (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255)) {		
		buffer[0] = 1; // ReportID
		buffer[1] = r;
		buffer[2] = g;
		buffer[3] = b;
		dataSize = 4;
		wValue = 1;
	} else if (sscanf(strcfg,"setLedColor %d:%d:%d:%d", &led, &r, &g, &b) == 4 
			&& (r >= 0 && r <= 255) && (g >= 0 && g <= 255) && (b >= 0 && b <= 255) && (led >= 0 && led <= LED_SIZE)) {
		buffer[0] = 2; // ReportID
		buffer[1] = led;
		buffer[2] = r;
		buffer[3] = g;
		buffer[4] = b;
		dataSize = 5;
		wValue = 2;
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

static ssize_t ledParty_read(struct file *file, char *user_buffer,
			  				size_t count, loff_t *ppos) {
	struct usb_ledParty *dev;
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

	wValue = 0x2;
	msgSize = 4;

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

	sprintf(message, "R: %d G: %d B: %d \n", message[1], message[2], message[3]);
	nr_bytes = strlen(message);

	if (copy_to_user(user_buffer, message, strlen(message))){
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
static const struct file_operations ledParty_fops = {
	.owner =	THIS_MODULE,
	.write =	ledParty_write,	 	/* write() operation on the file */
	.read =		ledParty_read,			/* read() operation on the file */
	.open =		ledParty_open,			/* open() operation on the file */
	.release =	ledParty_release, 		/* close() operation on the file */
};

/* 
 * Return permissions and pattern enabling udev 
 * to create device file names under /dev
 * 
 * For each ledPartystick connected device a character device file
 * named /dev/usb/ledPartystick<N> will be created automatically  
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
static struct usb_class_driver ledParty_class = {
	.name =		"ledParty%d",  /* Pattern used to create device files */	
	.devnode=	set_device_permissions,	
	.fops =		&ledParty_fops,
	.minor_base =	USB_BLINK_MINOR_BASE,
};

/*
 * Invoked when the USB core detects a new
 * ledPartystick device connected to the system.
 */
static int ledParty_probe(struct usb_interface *interface,
		      			const struct usb_device_id *id) {
	struct usb_ledParty *dev;
	int retval = -ENOMEM;

	/*
 	 * Allocate memory for a usb_ledParty structure.
	 * This structure represents the device state.
	 * The driver assigns a separate structure to each ledPartystick device
 	 *
	 */
	dev = kmalloc(sizeof(struct usb_ledParty), GFP_KERNEL);

	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}

	/* Initialize the various fields in the usb_ledParty structure */
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &ledParty_class);
	if (retval) {
		/* something prevented us from registering this driver */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/* let the user know what node this device is now attached to */	
	dev_info(&interface->dev,
		 "ledParty now available via ledParty%d",
		 interface->minor);
	return 0;

error:
	if (dev)
		/* this frees up allocated memory */
		kref_put(&dev->kref, ledParty_delete);
	return retval;
}

/*
 * Invoked when a ledPartystick device is 
 * disconnected from the system.
 */
static void ledParty_disconnect(struct usb_interface *interface) {
	struct usb_ledParty *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &ledParty_class);

	/* prevent more I/O from starting */
	dev->interface = NULL;

	/* decrement our usage count */
	kref_put(&dev->kref, ledParty_delete);

	dev_info(&interface->dev, "ledParty device #%d has been disconnected", minor);
}

/* Define these values to match your devices */
#define BLINKSTICK_VENDOR_ID	0X20A0
#define BLINKSTICK_PRODUCT_ID	0X41E5

/* table of devices that work with this driver */
static const struct usb_device_id ledParty_table[] = {
	{ USB_DEVICE(BLINKSTICK_VENDOR_ID,  BLINKSTICK_PRODUCT_ID) },
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, ledParty_table);

static struct usb_driver ledParty_driver = {
	.name =		"ledParty",
	.probe =	ledParty_probe,
	.disconnect =	ledParty_disconnect,
	.id_table =	ledParty_table,
};

/* Module initialization */
int ledPartydrv_module_init(void) {
   return usb_register(&ledParty_driver);
}

/* Module cleanup function */
void ledPartydrv_module_cleanup(void) {
  usb_deregister(&ledParty_driver);
}

module_init(ledPartydrv_module_init);
module_exit(ledPartydrv_module_cleanup);