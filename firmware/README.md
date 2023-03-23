# Custom Firmware Instructions

## Compiling and flashing the firmware
```bash
make clean # Clean workspace
make hex # Compile source code into an hex file
sudo make flash # Flashes the ATTiny85 using avrdude. See details below
```

In order to make use of the `make flash` command the ATTiny85 has to be wired to an USBASP programmer and you'll need to have an Arduino IDE installed under the paths specified in the Makefile.

### USBASP wiring diagram

| USBASP | ATTiny85 |
| ------ | -------- |
| VCC    | VIN      |
| GND    | GND      |
| MOSI   | PB0      |
| MISO   | PB1      |
| SCK    | PB2      |
| RST    | PB5      |

## Functionality

### Read ReportIDs

All read transaction have the first byte set to the correspondent report ID.

### Report ID 1

Returns the current values for R,G and B variables. 4 bytes total including report ID.

### Report ID 2

Returns a 33 bytes payload.

### Write ReportIDs
### Report ID 1

Changes all leds to the given color.

### Report ID 2

Not used

### Report ID 3

Changes specified led to specified color.

### Report ID 4

Displays hex character on 7 segment display

## Userspace PwnedDevice Module

pwnedDevice is a Python 3 module that has a few functions built-in to allow communication with the device skipping the kernel coding stage for fast prototyping.

### Read data example

```python
from pwnedDevice import pwnedDevice # Import module
b = pwnedDevice.find_first() # Find connected device
b.get_block(3, 33) # Sends READ request to ReportID 3 asking for 33 bytes of data.

```



## Notes of interest

### usbFunctionSetup()

This function is called on every SETUP request, prior to usbFunctionRead/usbFunctionWrite.

### usbFunctionRead()

This function is called when `usbFunctionSetup` returns `USB_NO_MSG` on a `USBRQ_HID_GET_REPORT` request.

It is called in chunks of up to 8 bytes each. It should copy the data to the location provided by `data` argument and returns the amount of bytes copied. If the return value is less than `len`, the transfer is terminated, if the value returned is `0xff` the drivers abort the transfer with a STALL token.

### Custom Requests

Custom requests can be defined on a new filled called `requests.h` that is shared with the host at the beginning of the SETUP. These requests should be handled on `usbFunctionSetup()`.