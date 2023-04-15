/*
 * Basic Driver to showcase how an INTERRUPT IN endpoint
 * can be access using the async USB API.
 *
 * Copyright (C) 2023 (Guillermo Gascón y Javier Rodriguez-Avello)
 *
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
#define USB_PWNED_DEVICE_MINOR_BASE	0 
#define BUFFER_SIZE 50

/* Structure to hold all of our device specific stuff */
struct usb_pwnedDevice {
	struct usb_device				*udev;			/* the usb device for this device */
	struct usb_interface			*interface;		/* the interface for this device */
	struct urb 						*int_in_urb;
	struct usb_endpoint_descriptor 	*int_in_endpoint;
	char int_in_buffer[BUFFER_SIZE];
	struct kref kref;
};
#define to_pwnedDevice_dev(d) container_of(d, struct usb_pwnedDevice, kref)

static struct usb_driver pwnedDevice_driver;

/*
* Callback function. Executed when INT IN URB returns back with data.
*/
static void pwnedDevice_int_in_callback(struct urb *urb) {
	struct usb_pwnedDevice *dev = urb->context;

	if (urb->status) {
		if (urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN) {
			printk(KERN_INFO "Error on callback!");
			return;
		}
	}

	if (urb->actual_length > 0)
		printk(KERN_INFO "Transfered data: %s\n", dev->int_in_buffer);	
}


/* 
 * Free up the usb_pwnedDevice structure and
 * decrement the usage count associated with the usb device 
 */
static void pwnedDevice_delete(struct kref *kref)
{
	struct usb_pwnedDevice *dev = to_pwnedDevice_dev(kref);

	usb_put_dev(dev->udev);
	kfree(dev);
}

/* Called when a user program invokes the open() system call on the device */
static int pwnedDevice_open(struct inode *inode, struct file *file)
{
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

	/* Initialize URB */
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
					usb_rcvintpipe(dev->udev,
								dev->int_in_endpoint->bEndpointAddress),
					dev->int_in_buffer,
					le16_to_cpu(0x0008),
					pwnedDevice_int_in_callback,
					dev,
					dev->int_in_endpoint->bInterval);
	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

	return retval;
}

/* Called when a user program invokes the close() system call on the device */
static int pwnedDevice_release(struct inode *inode, struct file *file)
{
	struct usb_pwnedDevice *dev;

	dev = file->private_data;

	if (dev->int_in_urb)
		usb_kill_urb(dev->int_in_urb);
		
	if (dev == NULL)
		return -ENODEV;

	/* decrement the count on our device */
	kref_put(&dev->kref, pwnedDevice_delete);

	return 0;
}

#define MAX_LEN_MSG 35
static ssize_t pwnedDevice_read(struct file *file, char *user_buffer,
			  size_t count, loff_t *ppos)
{
	struct usb_pwnedDevice *dev;
	int retval = 0;

	dev = file->private_data;
	
	if (*ppos > 0)
		return 0;

	/* Send URB. */
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
	                 usb_rcvintpipe(dev->udev,
	                                dev->int_in_endpoint->bEndpointAddress),
	                 dev->int_in_buffer,
	                 le16_to_cpu(0x0008),
	                 pwnedDevice_int_in_callback,
	                 dev,
	                 dev->int_in_endpoint->bInterval);
	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);

	if (retval != 0)
		return retval;

	return 0; // No bytes returned. Async USB API used.
}

/*
 * Operations associated with the character device 
 * exposed by driver
 * 
 */
static const struct file_operations pwnedDevice_fops = {
	.owner =	THIS_MODULE,
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
static struct usb_class_driver pwnedDevice_class = {
	.name =		"pwnedDevice-%d",  /* Pattern used to create device files */	
	.devnode=	set_device_permissions,	
	.fops =		&pwnedDevice_fops,
	.minor_base =	USB_PWNED_DEVICE_MINOR_BASE,
};

/*
 * Invoked when the USB core detects a new
 * pwnedDevicestick device connected to the system.
 */
static int pwnedDevice_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_pwnedDevice *dev;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int retval = -ENOMEM;
	int i;

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

	iface_desc = interface->cur_altsetting;

	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
		     == USB_DIR_IN)
		    && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		        == USB_ENDPOINT_XFER_INT))
			dev->int_in_endpoint = endpoint;
	}

	if (! dev->int_in_endpoint) {
		pr_err("could not find interrupt in endpoint");
		goto error;
	}


	/* Request IN URB */
	dev->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->int_in_urb) {
		printk(KERN_INFO "Error allocating URB");
		retval = -ENOMEM;
		goto error;
	}

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
		 "PwnedDevice now available via pwnedDevice-%d",
		 interface->minor);
	return 0;

error:
	if (dev->int_in_urb)
		usb_free_urb(dev->int_in_urb);

	if (dev)
		/* this frees up allocated memory */
		kref_put(&dev->kref, pwnedDevice_delete);

	return retval;
}

/*
 * Invoked when a pwnedDevicestick device is 
 * disconnected from the system.
 */
static void pwnedDevice_disconnect(struct usb_interface *interface)
{
	struct usb_pwnedDevice *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);

	if (dev->int_in_urb)
		usb_free_urb(dev->int_in_urb);

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
#define PWNED_DEVICESTICK_VENDOR_ID	0X20A0
#define PWNED_DEVICESTICK_PRODUCT_ID	0X41E5

/* table of devices that work with this driver */
static const struct usb_device_id pwnedDevice_table[] = {
	{ USB_DEVICE(PWNED_DEVICESTICK_VENDOR_ID,  PWNED_DEVICESTICK_PRODUCT_ID) },
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
int pwnedDevice_module_init(void)
{
   return usb_register(&pwnedDevice_driver);
}

/* Module cleanup function */
void pwnedDevice_module_cleanup(void)
{
  usb_deregister(&pwnedDevice_driver);
}

module_init(pwnedDevice_module_init);
module_exit(pwnedDevice_module_cleanup);

