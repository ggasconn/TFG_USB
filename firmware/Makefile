# Name: Makefile
# Project: hid-custom-rq example
# Author: Christian Starkjohann
# Creation Date: 2008-04-07
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
# This Revision: $Id: Makefile 692 2008-11-07 15:07:40Z cs $

OS := $(shell uname)

# Available values: ALL_DEVICES, LED_PARTY, DISPLAYS, PWM
TEMPLATE = ALL_DEVICES

ifeq ($(OS), Linux)
	AVR_BIN_PATH = /opt/arduino/hardware/tools/avr/bin/
	# AVR_BIN_PATH = /usr/bin/
else
	AVR_BIN_PATH = /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin
endif

#DEVICE  = attiny45
DEVICE  = atmega328p
DEVICE_ID = 1 #edit this to 0 for using attiny
#F_CPU   = 16500000
#FUSE_L  = 0xe1
#FUSE_H  = 0xdd

F_CPU   = 16000000
# Fuse L
# CKDIV = 1  not divide by 8
# CKOUT = 0  not output clock on port b
# SUT1, SUT0 = 11 16K CK on power down + 14 CK + 65 ms on reset
# CKSEL = 1111, low power crystal oscillator + Crystal oscillator, slowly rising
# power
FUSE_L  = 0xff
# Fuse H
# RSTDISBL = 1, not disable external reset (default value)
# DWEN = 1 (defualt value)
# SPIEN = 0, enable serial program and data downlosding (default value)
# WDTON = 1, watchdog timer always on unprogrammed (defual value)
# EESAVE = 1, EEPROM memory preserved through chip erase unprogrammed (default)
# BOOTSZ1, BOOTSZ0 = 00, 2048 words (not sure)
# BOOTRST = 1 (default)
FUSE_H  = 0xd9
#PROGRAMM_COMMAND = $(AVR_BIN_PATH)/avrdude -c usbtiny -P usb -p $(DEVICE) -C $(AVR_BIN_PATH)/../etc/avrdude.conf # edit this line for your programmer
PROGRAMM_COMMAND = $(AVR_BIN_PATH)/avrdude -c usbasp -P usb -p $(DEVICE) -C $(AVR_BIN_PATH)/../etc/avrdude.conf # edit this line for your programmer
#PROGRAMM_COMMAND = /Users/jcsaez/Library/Arduino15/packages/digistump/tools/micronucleus/2.0a4/launcher -cdigispark --timeout 30 # edit this line for your programmer

CFLAGS  = -Iusbdrv -I. -DDEBUG_LEVEL=1 -Wno-narrowing

ifeq ($(TEMPLATE), ALL_DEVICES)
	OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o light_ws2812/light_ws2812.o oled/oled.h display7S/display7S.h utils/utils.o
else ifeq ($(TEMPLATE), LED_PARTY)
	OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o light_ws2812/light_ws2812.o utils/utils.o
else ifeq ($(TEMPLATE), DISPLAYS)
	OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o oled/oled.h display7S/display7S.h utils/utils.o
else ifeq ($(TEMPLATE), PWM)
	OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o utils/utils.o
endif

COMPILE = $(AVR_BIN_PATH)/avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE) -include deviceconfig.h
COMPILEPP = $(AVR_BIN_PATH)/avr-g++ -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE) -include deviceconfig.h

# symbolic targets:
help:
	@echo "This Makefile has no default rule. Use one of the following:"
	@echo "make hex ....... to build main.hex"
	@echo "make program ... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware (use this on metaboard)"
	@echo "make dump ...... dump eeprom"
	@echo "make dumpflash . dump flash"
	@echo "make defaults .. write default eeprom"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make deploy .... program, increment serial, defaults"

hex: main.hex

program: flash fuse

# rule for programming fuse bits:
fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(PROGRAMM_COMMAND) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m

# rule for uploading firmware:
flash: main.hex
	$(PROGRAMM_COMMAND) -U flash:w:main.hex:i

dump: 
	$(PROGRAMM_COMMAND) -U eeprom:r:blinkstick-eeprom.hex:i

dumpflash: 
	$(PROGRAMM_COMMAND) -U flash:r:blinkstick-flash.hex:i

defaults: 
	$(PROGRAMM_COMMAND) -B 3 -U eeprom:w:eeprom.hex:i

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f main.hex main.lst main.obj main.cof main.list main.map main.eep.hex main.elf 
	rm -f main.o usbdrv/oddebug.o usbdrv/usbdrv.o usbdrv/usbdrvasm.o main.s usbdrv/oddebug.s usbdrv/usbdrv.s
	rm -f light_ws2812/light_ws2812.o
	rm -f dht/DHT.o
	rm -f utils/utils.o

.cpp.o:
	$(COMPILEPP) -c $< -o $@


# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

# Generic rule for compiling C to assembler, used for debugging only.
.c.s:
	$(COMPILE) -S $< -o $@

# file targets:

# Since we don't want to ship the driver multipe times, we copy it into this project:
usbdrv:
	cp -r ../../../usbdrv .

main.elf: usbdrv $(OBJECTS)	# usbdrv dependency only needed because we copy it
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex main.eep.hex
	$(AVR_BIN_PATH)/avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	$(AVR_BIN_PATH)/avr-size main.hex

# debugging targets:

disasm:	main.elf
	$(AVR_BIN_PATH)/avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.cpp

increment:
	ruby increment.rb

deploy: program increment defaults
