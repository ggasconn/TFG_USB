#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define NR_BYTES_PWNED_DEVICE_MSG 0x0021
#define USB_PWNED_DEVICE_MINOR_BASE	0

struct usb_pwnedDevice {
	struct usb_device	*udev;			/* the usb device for this device */
	struct usb_interface	*interface;		/* the interface for this device */
	struct kref		kref;
	struct urb *int_in_urb;
	struct urb *int_out_urb;
	struct usb_endpoint_descriptor *int_in_endpoint;
	struct usb_endpoint_descriptor *int_out_endpoint;
	char int_in_buffer[NR_BYTES_PWNED_DEVICE_MSG];
	char int_out_buffer[NR_BYTES_PWNED_DEVICE_MSG];
	spinlock_t lock;
};
#define to_pwnedDevice_dev(d) container_of(d, struct usb_pwnedDevice, kref)

static struct usb_driver pwnedDevice;


static void pwnedDevice_int_out_callback(struct urb *urb) {
	printk(KERN_INFO "Inside callback");
	
	if (urb->status)
		if (urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN)
			trace_printk("Error submitting urb\n");
}


static void pwnedDevice_int_in_callback(struct urb *urb) {
	// struct usb_pwnedDevice *dev = urb->context;
	int retval = 0;

	if (urb->status) {
		if (urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN) {
			return;
		} else {
			goto resubmit; /* Maybe we can recover. */
		}
	}

resubmit:
	retval = 0;//usb_submit_urb(dev->int_in_urb, GFP_ATOMIC);

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

	printk(KERN_INFO "I'm here!");

	/* Obtain reference to USB interface from minor number */
	interface = usb_find_interface(&pwnedDevice, subminor);
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

	if (! (file->f_mode & FMODE_READ))
		return 0;

	/* Initialize interrupt URB. */
	usb_fill_int_urb(dev->int_in_urb, dev->udev,
	                 usb_sndintpipe(dev->udev,
	                                dev->int_in_endpoint->bEndpointAddress),
	                 dev->int_in_buffer,
	                 le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize),
	                 pwnedDevice_int_in_callback,
	                 dev,
	                 dev->int_in_endpoint->bInterval);
	retval = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);

	if (retval!=0)
		return retval;

	printk(KERN_INFO ">> Device opened successfully");

	return 0;
}

/* Called when a user program invokes the close() system call on the device */
static int pwnedDevice_release(struct inode *inode, struct file *file)
{
	struct usb_pwnedDevice *dev;

	dev = file->private_data;
	if (dev == NULL)
		return -ENODEV;

	if (file->f_mode & FMODE_READ) {
		if (dev->int_in_urb) {
			usb_kill_urb(dev->int_in_urb);
		}
	}

	/* decrement the count on our device */
	kref_put(&dev->kref, pwnedDevice_delete);

	return 0;
}

/* Called when a user program invokes the write() system call on the device */
static ssize_t pwnedDevice_write(struct file *file, const char *user_buffer,
                            size_t len, loff_t *off)
{
	int ret = 0;
	unsigned char message[33];
	int actual_len=0;
	struct usb_pwnedDevice *dev=file->private_data;

	/* zero fill*/
	memset(message,0,33);

	/* Fill up the message accordingly */
	message[0]='X';

	/* Send message (URB) */
	ret=usb_interrupt_msg(dev->udev,
	                      usb_sndintpipe(dev->udev,dev->int_in_endpoint->bEndpointAddress),
	                      message,	/* Pointer to the message */
	                      33, /* message's size in bytes */
	                      &actual_len,
	                      1000);

	if (ret < 0) {
		trace_printk("Couldn't send packet\n");
		return ret;
	} else if (actual_len != 33)
		return -EIO;

	(*off) += len;
	return len;
}


/*
 * Operations associated with the character device
 * exposed by driver
 *
 */
static const struct file_operations pwnedDevice_fops = {
	.owner =	THIS_MODULE,
	.write =	pwnedDevice_write,	 	/* write() operation on the file */
	.open =		pwnedDevice_open,			/* open() operation on the file */
	.release =	pwnedDevice_release, 		/* close() operation on the file */
};

/*
 * Return permissions and pattern enabling udev
 * to create device file names under /dev
 *
 * For each pwnedDevice connected device a character device file
 * named /dev/usb/pwnedDevice<N> will be created automatically
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
	.name =		"pwnedDevice%d",  /* Pattern used to create device files */
	.devnode=	set_device_permissions,
	.fops =		&pwnedDevice_fops,
	.minor_base =	USB_PWNED_DEVICE_MINOR_BASE,
};
 
static int pen_probe(struct usb_interface *interface, const struct usb_device_id *id)
{

    struct usb_pwnedDevice *dev;
	int retval = -ENOMEM;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i, int_end_size;

    /*
	 * Allocate memory for a usb_pwnedDevice structure.
	 * This structure represents the device state.
	 * The driver assigns a separate structure to each pwnedDevice device
	 *
	 */
	dev = kzalloc(sizeof(struct usb_pwnedDevice),GFP_KERNEL);

	if (!dev) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error;
	}

    // printk(KERN_INFO "Hey");

	/* Initialize the various fields in the usb_pwnedDevice structure */
	kref_init(&dev->kref);
	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

    iface_desc = interface->cur_altsetting;

	/* Set up interrupt endpoint information. */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
		     == USB_DIR_IN)
		    && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		        == USB_ENDPOINT_XFER_INT))
			dev->int_in_endpoint = endpoint;
        
		
        if (((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
		     == USB_DIR_OUT)
		    && ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		        == USB_ENDPOINT_XFER_INT))
			dev->int_out_endpoint = endpoint;
		
	}

    if (! dev->int_in_endpoint) {
       pr_err("could not find interrupt in endpoint");
		goto error;
	}

    int_end_size = le16_to_cpu(dev->int_in_endpoint->wMaxPacketSize);

    dev->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (! dev->int_in_urb) {
		retval = -ENOMEM;
		goto error;
	}

	
    dev->int_out_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (! dev->int_out_urb) {
		retval = -ENOMEM;
		goto error;
	}

	
    memset(dev->int_out_buffer,0,NR_BYTES_PWNED_DEVICE_MSG);
	dev->int_out_buffer[0] = 0x00;

	printk(KERN_INFO "UDEV: %p", dev->udev);
	printk(KERN_INFO "bEndpointAddress: %d", dev->int_out_endpoint->bEndpointAddress);

	// Initialize interrupt URB.
	usb_fill_int_urb(dev->int_out_urb, dev->udev,
	                 usb_sndintpipe(dev->udev,
	                                dev->int_out_endpoint->bEndpointAddress),
	                 dev->int_out_buffer,
	                 NR_BYTES_PWNED_DEVICE_MSG,
	                 pwnedDevice_int_out_callback,
	                 dev,
	                 dev->int_out_endpoint->bInterval);

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
	         "PwnedDevice now attached to pwnedDevice-%d",
	         interface->minor);

    printk(KERN_INFO "PwnedDevice (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
    return 0;

error:
	printk(KERN_INFO "Error goto");

	if (dev->int_in_urb)
		usb_free_urb(dev->int_in_urb);

	if (dev->int_out_urb)
		usb_free_urb(dev->int_out_urb);
	
	if (dev)
		kref_put(&dev->kref, pwnedDevice_delete);

	return retval;
}
 
static void pen_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "PwnedDevice removed\n");
}

#define PWNED_DEVICE_VENDOR_ID	0X20A0
#define PWNED_DEVICE_PRODUCT_ID	0X41E5

static struct usb_device_id pen_table[] =
{
    { USB_DEVICE(PWNED_DEVICE_VENDOR_ID, PWNED_DEVICE_PRODUCT_ID) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, pen_table);
 
static struct usb_driver pwnedDevice =
{
    .name = "pwnedDevice",
    .id_table = pen_table,
    .probe = pen_probe,
    .disconnect = pen_disconnect,
};
 
static int __init pen_init(void)
{
    return usb_register(&pwnedDevice);
}
 
static void __exit pen_exit(void)
{
    usb_deregister(&pwnedDevice);
}
 
module_init(pen_init);
module_exit(pen_exit);
 
MODULE_LICENSE("GPL");