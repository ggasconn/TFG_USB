#include <stdio.h> // printf
#include <wchar.h> // wprintf
#include <unistd.h> // sleep

#include <hidapi.h>

#define NR_LEDS 12

/**
 * COMPILE SENTENCE
 *   gcc userProgram.c -o userProgram -I/usr/include/hidapi/ -lhidapi-libusb 
 */

int main(int argc, char* argv[]) {
	unsigned char buf[5];
	hid_device *handle;
	unsigned int i;

	// Initialize the hidapi library
	hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x20a0, 0x41e5, NULL);

	// Cycle through RGB and turn off all leds
	buf[0] = 0x1; buf[1] = 255; buf[2] = 0; buf[3] = 0;
    hid_write(handle, buf, 4);

	sleep(1);

	buf[0] = 0x1; buf[1] = 0; buf[2] = 255; buf[3] = 0;
    hid_write(handle, buf, 4);

	sleep(1);

	buf[0] = 0x1; buf[1] = 0; buf[2] = 0; buf[3] = 255;
    hid_write(handle, buf, 4);

	sleep(1);

	for (i = 0; i < NR_LEDS; i++) {
		buf[0] = 0x2; buf[1] = i; buf[2] = 255; buf[3] = 0; buf[4] = 255;
    	hid_write(handle, buf, 5);
		sleep(1);
	}
	
	buf[0] = 0x1; buf[1] = 0; buf[2] = 0; buf[3] = 0;
    hid_write(handle, buf, 4);
	
	// Close the device
	hid_close(handle);

	// Finalize the hidapi library
	hid_exit();

	return 0;
}
