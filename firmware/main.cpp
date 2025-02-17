#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>	/* for sei() */
#include <string.h>
#include <stdio.h>

#include <avr/pgmspace.h>	/* required by usbdrv.h */
#include <stdint.h>

extern "C" {
	#include "usbdrv.h"
	#include "oddebug.h"		/* This is also an example for using debug macros */
	
	#if LED_PARTY == 1
	#include "light_ws2812/light_ws2812.h"
	#endif
	
	#if DISPLAYS == 1
	#include "oled/oled.h"
	#include "display7S/display7S.h"
	#endif

	#include "utils/utils.h"
	#include "deviceconfig.h"
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

// If descriptor changes, USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH also has to be updated in usbconfig.h

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {    /* USB report descriptor */
	0x06, 0x00, 0xff,			   // USAGE_PAGE (Generic Desktop)
	0x09, 0x01,					   // USAGE (Vendor Usage 1)
	0xa1, 0x01,					   // COLLECTION (Application)
	0x15, 0x00,					   //	LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,			   //	LOGICAL_MAXIMUM (255)
	0x75, 0x08,					   //	REPORT_SIZE (8)
	0x85, 0x01,					   //	REPORT_ID (1)
	0x95, 0x03,					   //	REPORT_COUNT (3)
	0x09, 0x00,					   //	USAGE (Undefined)
	0xb2, 0x02, 0x01,			   //	FEATURE (Data,Var,Abs,Buf)

	0x85, 0x02,					   //	REPORT_ID (2)
	0x95, 0x20,					   //	REPORT_COUNT (32)
	0x09, 0x00,					   //	USAGE (Undefined)
	0xb2, 0x02, 0x01,			   //	FEATURE (Data,Var,Abs,Buf)

	0x15, 0x00,					   //	LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,			   //	LOGICAL_MAXIMUM (255)
	0x75, 0x08,					   //	REPORT_SIZE (8)
	0x85, 0x03,					   //	REPORT_ID (3)
	0x95, 0x04,					   //	REPORT_COUNT (4)
	0x09, 0x00,					   //	USAGE (Undefined)
	0xb2, 0x02, 0x01,			   //	FEATURE (Data,Var,Abs,Buf)
	0xc0						   // END_COLLECTION
};

/* ------------------------------------------------------------------------- */
/* --------------------------- Global Variables ---------------------------- */
/* ------------------------------------------------------------------------- */

#define NUMBER_OF_LEDS	 12
#define INTERRUPT_TRANSFER_SIZE  8

struct cRGB ledStatus[NUMBER_OF_LEDS];

#define MSG "Hello, World! I'm pwnedDevice ;)"

#define SERIAL_NUMBER_STR "PWND-00000.1"
uchar INT_IN_MSG[INTERRUPT_TRANSFER_SIZE] = "Hello;)";

static volatile uint8_t r = 0;
static volatile uint8_t g = 0;
static volatile uint8_t b = 0;

static uchar currentAddress;
static uchar bytesRemaining;
static uchar reportId = 0;

static uchar replyBuffer[33]; // 32 for data + 1 for report id


/* ------------------------------------------------------------------------- */
/* ----------------------------- USB Functions ----------------------------- */
/* ------------------------------------------------------------------------- */

/* usbFunctionRead() is called when the host requests a chunk of data from
* the device.
*/
uchar usbFunctionRead(uchar *data, uchar len) {
	if (reportId == 1) {
		if(len > bytesRemaining)
			len = bytesRemaining;

		if (currentAddress == 0) {
			memcpy(&data[1], &MSG[currentAddress], len);
			currentAddress += (len - 1);
			bytesRemaining -= (len - 1);
		} else {
			memcpy(data, &MSG[currentAddress], len);
			currentAddress += len;
			bytesRemaining -= len;
		}

		return len;
	}

	return 0;
}

/* usbFunctionWrite() is called when the host sends a chunk of data to the
* device.
*/
uchar usbFunctionWrite(uchar *data, uchar len) {
	#if LED_PARTY == 1
	if (reportId == 1) {
		//Only send data if the color has changed
		if (data[1] != r || data[2] != g || data[3] != b) {
			r = data[1];
			g = data[2];
			b = data[3];

			// switch color order "G,R,B"
			uint8_t led_data[NUMBER_OF_LEDS * 3];
			for (uint16_t i=0; i<NUMBER_OF_LEDS; i++) {
				led_data[i*3 + 0] = data[2];
				led_data[i*3 + 1] = data[1];
				led_data[i*3 + 2] = data[3];
			}

			// cli(); //Disable interrupts
			ws2812_sendarray_mask(&led_data[0], sizeof(led_data), _BV(ws2812_pin));
			// sei(); //Enable interrupts
		}

		return 1;
	} else	if (reportId == 2) {
		// switch color order "G,R,B"
		ledStatus[data[1]].g = data[3];
		ledStatus[data[1]].r = data[2];
		ledStatus[data[1]].b = data[4];

		// cli(); //Disable interrupts
		ws2812_setleds(&ledStatus[0], sizeof(ledStatus));
		// sei(); //Enable interrupts

		return 1;
	}
	#endif

	#if DISPLAYS == 1
	if (reportId == 3) {
		if (len > bytesRemaining)
			len = bytesRemaining;

		// Store DDRB and PORTB values for later restore
		unsigned char DDRB_OLD = DDRB;
		unsigned char PORTB_OLD = PORTB;

		OLED_init();

		// This function is called in chunks of 8 bytes,
		// so if we are printing the first 8 bytes screen
		// should be cleared
		if (currentAddress == 0)
			OLED_clearScreen();

		// Print char by char
		for (uint16_t i = 0; i < len; i++)
			OLED_print(data[i]);

		currentAddress += len;

		DDRB = DDRB_OLD;
		PORTB = PORTB_OLD;

		return 1;
	} else if (reportId == 4) {
		display7sSet(data[1]);

		return 1;
	} 
	#endif

	#if PWM == 1
	if (reportId == 5) {
		uchar stopTimer = data[1];

		if (stopTimer)
			clearTimer1();
		else 
			blinkPWM();

		return 1;
	} else if (reportId == 6) {
		uchar stopTimer = data[1];

		if (stopTimer)
			clearTimer1();
		else {
			long int frequency = (data[2] << 24) | (data[3] << 16) | (data[4] << 8) | data[5];
			
			hardwarePWMBeep(frequency);
		}

		return 1;
	}
	#endif

	return 1;
}

#define SERIAL_NUMBER_LENGTH 12 // BSXXXXXX-1.0
static int	serialNumberDescriptor[SERIAL_NUMBER_LENGTH + 1];

/* Sends the serial number when requested */
uchar usbFunctionDescriptor(usbRequest_t *rq) {
   uchar len = 0;
   usbMsgPtr = 0;
   if (rq->wValue.bytes[1] == USBDESCR_STRING && rq->wValue.bytes[0] == 3) // 3 is the type of string descriptor, in this case the device serial number
   {
	  usbMsgPtr = (uchar*)serialNumberDescriptor;
	  len = sizeof(serialNumberDescriptor);
   }
   return len;
}

/* Retrieves the serial number from EEPROM */
static void SetSerial(void) {
   serialNumberDescriptor[0] = USB_STRING_DESCRIPTOR_HEADER(SERIAL_NUMBER_LENGTH);

   uchar serialNumber[SERIAL_NUMBER_LENGTH];
   memcpy(serialNumber, SERIAL_NUMBER_STR, SERIAL_NUMBER_LENGTH);

   for (int i =0; i < SERIAL_NUMBER_LENGTH; i++)
	serialNumberDescriptor[i + 1] = serialNumber[i];
}

/* ------------------------------------------------------------------------- */

extern "C" usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *)data;
	reportId = rq->wValue.bytes[0];

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){ /* HID class request */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){ /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			 usbMsgPtr = replyBuffer;

			if (reportId == 1) {
				odPrintf("Received GET on ReportID 1\n");

				replyBuffer[0] = 2; // report id

				bytesRemaining = 33;
				currentAddress = 0;

				return USB_NO_MSG; /* use usbFunctionRead() to obtain data */

			} 
			#if LED_PARTY == 1
			else if(reportId == 2) { // Device colors
				odPrintf("Received GET on ReportID 2\n");

				replyBuffer[0] = 1; // report id
				replyBuffer[1] = r;
				replyBuffer[2] = g;
				replyBuffer[3] = b;

				return 4; /* Return the data from this function */
			}
			#endif

			return 0;

		} else if(rq->bRequest == USBRQ_HID_SET_REPORT) {

			#if LED_PARTY == 1
			if (reportId == 1) { // Device colors
				odPrintf("Received SET on ReportID 1\n");

				bytesRemaining = 3;
				currentAddress = 0;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */

			} else if (reportId == 2) { // Set n LED to RGB color
				odPrintf("Received SET on ReportID 2\n");

				bytesRemaining = 4;
				currentAddress = 0;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */

			}
			#endif

			#if DISPLAYS == 1
			if (reportId == 3) {
				odPrintf("Received SET on ReportID 3\n");

				/* wLenght is the amount of bytes sent */
				bytesRemaining = rq->wLength.bytes[0];
				currentAddress = 0;

				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */

			} else if (reportId == 4) {
				odPrintf("Received SET on ReportID 4\n");

				bytesRemaining = 1;
				currentAddress = 0 ;

				return USB_NO_MSG;

			}
			#endif

			#if PWM == 1
			if (reportId == 5) {
				odPrintf("Received SET on ReportID 5\n");
				
				bytesRemaining = 2;
				currentAddress = 0;

				return USB_NO_MSG;

			} else if (reportId == 6) {
				odPrintf("Received SET on ReportID 6\n");
				
				bytesRemaining = 6;
				currentAddress = 0;

				return USB_NO_MSG;
			}
			#endif

			return 0;
		}
	}

	return 0;	/* default for not implemented requests: return no data back to host */
}

static void calibrateOscillator(void) {
	uchar		step = 128;
	uchar		trialValue = 0, optimumValue;
	int			x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

	/* do a binary search: */
	do{
		OSCCAL = trialValue + step;
		x = usbMeasureFrameLength();	// proportional to current real frequency
		if(x < targetValue)				// frequency still too low
			trialValue += step;
		step >>= 1;
	}while(step > 0);
	/* We have a precision of +/- 1 for optimum OSCCAL here */
	/* now do a neighborhood search for optimum value */
	optimumValue = trialValue;
	optimumDev = x; // this is certainly far away from optimum
	for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
		x = usbMeasureFrameLength() - targetValue;
		if(x < 0)
			x = -x;
		if(x < optimumDev){
			optimumDev = x;
			optimumValue = OSCCAL;
		}
	}
	OSCCAL = optimumValue;
}

extern "C" void usbEventResetReady(void) {
	cli();	// usbMeasureFrameLength() counts CPU cycles, so disable interrupts.
	calibrateOscillator();
	sei();
	eeprom_write_byte(0, OSCCAL);	// store the calibrated value in EEPROM
}

/* ------------------------------------------------------------------------- */

int main(void) {

	wdt_enable(WDTO_1S);

	SetSerial();

	/* Even if you don't use the watchdog, turn it off here. On newer devices,
	 * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
	 */
	
	/* RESET status: all port bits are inputs without pull-up.
	 * That's the way we need D+ and D-. Therefore we don't need any
	 * additional hardware initialization.
	 */

	odDebugInit();
	odPrintf("V-USB device initializing... ");

	usbInit();
	sei();

	uchar i = 0;
	
	usbDeviceDisconnect();	// enforce re-enumeration, do this while interrupts are disabled!
	while(--i) {				// fake USB disconnect for > 250 ms
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	
	blinkledRx();

	#if LED_PARTY == 1
	// Initialize all leds to 0
	for (i = 0; i < NUMBER_OF_LEDS; i++)
		ledStatus[i] = { 0, 0, 0 };
	#endif

	odPrintf("OK!\n");

	for(;;){				/* main event loop */
		wdt_reset();
		usbPoll();

		/* Check if there's any INTERRUPT IN URB to fill in */
		if(usbInterruptIsReady())
			usbSetInterrupt(INT_IN_MSG, INTERRUPT_TRANSFER_SIZE); // Fill URB buffer with data;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */
