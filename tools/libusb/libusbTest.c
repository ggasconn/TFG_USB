#include <stdio.h>
#include <unistd.h> // sleep
#include <libusb-1.0/libusb.h>
#include <time.h>
#include "pitches.h"

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
                                    0x0001, // wValue
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
                                    0x0002, // wValue
                                    0, // wIndex
                                    data, // data
                                    5, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void readBlock(libusb_device_handle *handle, unsigned int reportID, unsigned int nBytes) {
    int res = -1;
    unsigned char data[33];
    int len = 0;

    //int ret = libusb_interrupt_transfer(handle, 0x81, data, sizeof(8), &len, 0);

    
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, // bmRequestType
                                    0x1,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0001, // wValue
                                    0, // wIndex
                                    data, // data
                                    nBytes, // wLength
                                    5000); // timeout

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

void setBeep(libusb_device_handle *handle, unsigned int reportID, int n1) {
    int res = -1;
    unsigned char data[6];

    data[0] = reportID;
    data[1] = 0;
    data[2] = (n1 >> 24) & 0xFF;
    data[3] = (n1 >> 16) & 0xFF;
    data[4] = (n1 >> 8) & 0xFF;
    data[5] = n1 & 0xFF;

    int num1 = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];
    int num2 = (data[5] << 24) | (data[6] << 16) | (data[7] << 8) | data[8];
    printf("Data1: %d Data2: %d \n", num1, num2);
    
    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0006, // wValue
                                    0, // wIndex
                                    data, // data
                                    6, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void stopBeep(libusb_device_handle *handle, unsigned int reportID) {
    int res = -1;
    unsigned char data[6];

    data[0] = reportID;
    data[1] = 1;

    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0006, // wValue
                                    0, // wIndex
                                    data, // data
                                    2, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void blinkLedPWM(libusb_device_handle *handle, unsigned int reportID) {
    int res = -1;
    unsigned char data[6];

    data[0] = reportID;
    data[1] = 0;

    res = libusb_control_transfer(handle, // device
                                    LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT, // bmRequestType
                                    0x9,  // bRequest 0x1 -> GET  0x9 -> SET
                                    0x0005, // wValue
                                    0, // wIndex
                                    data, // data
                                    2, // wLength
                                    5000); // timeout

    if (res < 0)
        printf(">>> [ERROR]: Cannot send URB! Code: %d\n", res);
}

void delay(unsigned int milliseconds) {
   clock_t start_time = clock();   // get the start time
   while (clock() < start_time + milliseconds);   // busy-wait loop
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
    sleep(1);
    setText(handle, 0x3);
    // readTemp(handle, 0x3, 4);
    sleep(1);
    
    int melody[] = {
    NOTE_C4, NOTE_C4, 
    NOTE_D4, NOTE_C4, NOTE_F4,
    NOTE_E4, NOTE_C4, NOTE_C4, 
    NOTE_D4, NOTE_C4, NOTE_G4,
    NOTE_F4, NOTE_C4, NOTE_C4,
    
    NOTE_C5, NOTE_A4, NOTE_F4, 
    NOTE_E4, NOTE_D4, NOTE_AS4, NOTE_AS4,
    NOTE_A4, NOTE_F4, NOTE_G4,
    NOTE_F4
    };

    int durations[] = {
    4, 8, 
    4, 4, 4,
    2, 4, 8, 
    4, 4, 4,
    2, 4, 8,
    
    4, 4, 4, 
    4, 4, 4, 8,
    4, 4, 4,
    2
    };
    
    int size = sizeof(durations) / sizeof(int);

    for (int note = 0; note < size; note++) {
        //to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int duration = 1000 / durations[note];
        setBeep(handle, 0x6, melody[note]);

        //to distinguish the notyes, set a minimum time between them.
        //the note's duration + 30% seems to work well:
        int pauseBetweenNotes = duration * 2.0;
        usleep(pauseBetweenNotes*1000);
        
        //stop the tone playing:
        stopBeep(handle, 0x6);
    }
    sleep(1);
    
    blinkLedPWM(handle, 0x5);
    sleep(1);

    //setLedColor(handle, 0x2, 2, 21, 43, 93);
    //sleep(1);
    //setColor(handle, 0x1, 223, 2, 91);
    /*
    sleep(1);
    sleep(1);
    */
    setLedColor(handle, 0x3, 4, 51, 243, 83);
    sleep(1);
    setLedColor(handle, 0x3, 6, 33, 123, 23);

    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(NULL);

    return 0;
}
