# Custom Firmware Instructions
## Flashing the firmware to the ATTiny85
Go to your arduino's installation folder and the execute the following command after compiling the firmware using the supplied Makefile:
```bash
hardware/tools/avr/bin/avrdude -Chardware/tools/avr/etc/avrdude.conf -v -pattiny85 -cusbasp -Pusb -Uflash:w:main.hex:i
```

## Compiling the user program
```bash
gcc userProgram.c -o userProgram -I/usr/include/hidapi/ -lhidapi-libusb 
```

## Wire the hardware
Connect VCC and GND from the ATTiny to the led ring and PB1 from the ATTiny to the DI pin on the led ring.

## Running the user program
```bash
sudo ./userProgram
```