# TFG_USB

## Directories

* `doc/`: documentation and tutorials
* `drivers/`: kernel drivers for handling the communication with the device
* `firmware/`: AVR code. Uses VUSB to emulate a low speed USB device.
* `tools/`: Custom tools, mainly designed for communicating with the device skipping the kernel coding stage

## Device

### Platform

This branch uses the ATMega328P microcontroller.

### Report  IDs

* **GET**

  * **Report ID 1**

    Returns the current values for R,G and B variables. 4 bytes total including report ID.

  * **Report ID 2**

    Returns a 33 bytes payload.

* **WRITE**

  * **Report ID 1**
    
    Changes all leds to the given color.
    
  * **Report ID 2**
    
    Changes specified led to specified color.
    
  * **Report ID 3**
    
    Writes on the OLED display the text sent on the payload.
    
  * **Report ID 4**

​				Displays the hexadecimal number sent on the 7S display.