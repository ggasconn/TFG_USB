#include <stdio.h>
#include <unistd.h> // sleep
#include <libusb-1.0/libusb.h>

#define MSG "Hello, World! :) Sent from the computer"

void setText(libusb_device_handle *handle, unsigned int reportID) {
    int res = -1;
    unsigned char data[41];

    data[0] = reportID;

    for (int i = 1; i < 40; i++)
        data[i] = MSG[i - 1];
    
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0003, // wValue
                                    0, // wIndex
                                    data, // data
                                    41, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void setColor(libusb_device_handle *handle, unsigned int reportID, unsigned int r, unsigned int g, unsigned int b) {
    int res = -1;
    unsigned char data[4];

    data[0] = reportID;
    data[1] = r;
    data[2] = g;
    data[3] = b;
    
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0201, // wValue
                                    0, // wIndex
                                    data, // data
                                    4, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void setLedColor(libusb_device_handle *handle, unsigned int reportID, unsigned int led, unsigned int r, unsigned int g, unsigned int b) {
    int res = -1;
    unsigned char data[5];

    data[0] = reportID;
    data[1] = led;
    data[2] = r;
    data[3] = g;
    data[4] = b;
    
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0202, // wValue
                                    0, // wIndex
                                    data, // data
                                    5, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void readBlock(libusb_device_handle *handle, unsigned int reportID, unsigned int nBytes) {
    int res = -1;
    unsigned char data[8];
    int len = 0;

    int ret = libusb_interrupt_transfer(handle, 0x81, data, sizeof(8), &len, 0);

    /*
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, // bmRequestType
                                    0x1,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0002, // wValue
                                    0, // wIndex
                                    data, // data
                                    nBytes, // wLength
                                    5000); // timeout
    */

    if (res < 0)
        printf("Error: Failed to read from endpoint 0 with report ID 2\n");
    else
        printf("Success: Read %d bytes from endpoint 0 with report ID 2. \n\t%s\n", res, data + 1); // Ignore first byte
}

void readTemp(libusb_device_handle *handle, unsigned int reportID, unsigned int nBytes) {
    int res = -1;
    unsigned char data[nBytes];

    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, // bmRequestType
                                    0x1,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0003, // wValue
                                    0, // wIndex
                                    data, // data
                                    nBytes, // wLength
                                    5000); // timeout

    //if (res < 0)
    //    printf("Error: Failed to read from endpoint 0 with report ID 2\n");
    //else
        printf("Success: Read %d bytes from endpoint 0 with report ID 2. \n\t%s\n", res, data + 1); // Ignore first byte
}



int main() {
    int result = -1;
    libusb_device_handle *handle;

    libusb_init(NULL);

    // Try to find and open device with PID:VID
    handle = libusb_open_device_with_vid_pid(NULL, 0x20a0, 0x41e5);
    if (!handle) {
        printf("Error: Could not open device\n");
        return 1;
    }
    
    result = libusb_detach_kernel_driver(handle, 0);
    if (result < 0 && result != LIBUSB_ERROR_NOT_FOUND && result != LIBUSB_ERROR_NOT_SUPPORTED) {
        fprintf(stderr, "Error detaching kernel driver: %s\n", libusb_error_name(result));
        return 1;
    }

    result = libusb_claim_interface(handle, 0);
    if (result < 0) {
        fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(result));
        return 1;
    }

    readBlock(handle, 0x2, 33);

    /*
    sleep(1);

    setLedColor(handle, 0x3, 2, 21, 43, 93);
    sleep(1);
    setLedColor(handle, 0x3, 4, 51, 243, 83);
    sleep(1);
    setLedColor(handle, 0x3, 6, 33, 123, 23);
    */

    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(NULL);

    return 0;
}
