# Custom Firmware Instructions

## Compiling and flashing the firmware
```bash
make clean # Clean workspace
make hex # Compile source code into an hex file
sudo make program # Flashes the microcontroller using avrdude. See details below
```

In order to make use of the `make flash` command the ATTiny85 has to be wired to an USBASP programmer and you'll need to have an Arduino IDE installed under the paths specified in the Makefile.

## Notes of interest

### usbFunctionSetup()

This function is called on every SETUP request, prior to usbFunctionRead/usbFunctionWrite.

### usbFunctionRead()

This function is called when `usbFunctionSetup` returns `USB_NO_MSG` on a `USBRQ_HID_GET_REPORT` request.

It is called in chunks of up to 8 bytes each. It should copy the data to the location provided by `data` argument and returns the amount of bytes copied. If the return value is less than `len`, the transfer is terminated, if the value returned is `0xff` the drivers abort the transfer with a STALL token.

### Custom Requests

Custom requests can be defined on a new filled called `requests.h` that is shared with the host at the beginning of the SETUP. These requests should be handled on `usbFunctionSetup()`.