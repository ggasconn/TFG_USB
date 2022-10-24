#define NUMBER_OF_LEDS   12  // because of hardware restrictions, 
                            // the maximum is around ca. 500... to be tested further

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */

extern "C"{
	#include "usbdrv.h"
}

#include "oddebug.h"        /* This is also an example for using debug macros */

extern "C" {
	#include "light_ws2812/light_ws2812.h"
}

struct cRGB ledStatus[NUMBER_OF_LEDS];

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

//if descriptor changes, USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH also has to be updated in usbconfig.h

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {    /* USB report descriptor */

    0x06, 0x00, 0xff,              // USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Vendor Usage 1)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x85, 0x01,                    //   REPORT_ID (1)
    0x95, 0x03,                    //   REPORT_COUNT (3)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)

    0x85, 0x02,                    //   REPORT_ID (2)
    0x95, 0x20,                    //   REPORT_COUNT (32)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)

    0x85, 0x03,                    //   REPORT_ID (3)
    0x95, 0x20,                    //   REPORT_COUNT (32)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)

   	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x85, 0x04,                    //   REPORT_ID (4)
    0x95, 0x04,                    //   REPORT_COUNT (4)
    0x09, 0x00,                    //   USAGE (Undefined)
    0xb2, 0x02, 0x01,              //   FEATURE (Data,Var,Abs,Buf)
    0xc0                           // END_COLLECTION
};

static volatile uint8_t r = 0;
static volatile uint8_t g = 0;
static volatile uint8_t b = 0;

static uchar currentAddress;
static uchar addressOffset;
static uchar bytesRemaining;
static uchar reportId = 0; 

static uchar replyBuffer[33]; // 32 for data + 1 for report id

/* usbFunctionRead() is called when the host requests a chunk of data from
* the device. 
*/
uchar usbFunctionRead(uchar *data, uchar len) {
	if (reportId == 1) {
		//Not used
		return 0;
	} else if (reportId == 2 || reportId == 3) {
		if(len > bytesRemaining)
			len = bytesRemaining;

		//Ignore the first byte of data as it's report id
		if (currentAddress == 0) {
			data[0] = reportId;
			eeprom_read_block(&data[1], (uchar *)0 + currentAddress + addressOffset, len - 1);
			currentAddress += len - 1;
			bytesRemaining -= (len - 1);
		} else {
			eeprom_read_block(data, (uchar *)0 + currentAddress + addressOffset, len);
			currentAddress += len;
			bytesRemaining -= len;
		}

		return len;
	} else
		return 0;
}


#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

static inline void blinkled(void){
	unsigned char oldstate;
	int i;
	oldstate=DDRB;
	DDRB|=(1<<D7S_DATA);

	/* Led test */
	for (i=0;i<3;i++){
		PORTB|=(0x1 <<  D7S_DATA);
		_delay_ms(100);
		PORTB&=~(1 <<  D7S_DATA);
		_delay_ms(100);
	}
	DDRB=oldstate;
}

static void display7sSet(unsigned char data){
	int i=0;
	int value=0;
	unsigned char oldpinstate=DDRB;

	//blinkled();
	/* Set pin mode */
	DDRB|=(1<<D7S_DATA)|(1<<D7S_SRCLK) | (1<<D7S_RCLK) ;

	for (i=0;i<8;i++){
		if (0x80 & (data << i))
			value=1;
		else
			value=0;

		// gpio_set_value(display_gpio[SDI_IDX], value);
		if (value)
			PORTB|=(0x1 <<  D7S_DATA);
		else
			PORTB&=~(1 <<  D7S_DATA);
		//gpio_set_value(display_gpio[SRCLK_IDX], 1);
		PORTB|=	(1 <<  D7S_SRCLK);
		_delay_ms(10);
		//gpio_set_value(display_gpio[SRCLK_IDX], 0);
		PORTB&=~(1 <<  D7S_SRCLK);
		_delay_ms(10);
	}

	//gpio_set_value(display_gpio[RCLK_IDX], 1);
	PORTB|=	1 <<  D7S_RCLK;
	_delay_ms(10);
	//	gpio_set_value(display_gpio[RCLK_IDX], 0);
	PORTB&=~(1 <<  D7S_RCLK);

	/* Restore pin mode */
	DDRB=oldpinstate;
}


/* usbFunctionWrite() is called when the host sends a chunk of data to the
* device. 
*/
uchar usbFunctionWrite(uchar *data, uchar len) {
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

			cli(); //Disable interrupts
			ws2812_sendarray_mask(&led_data[0], sizeof(led_data), _BV(ws2812_pin));
			sei(); //Enable interrupts
		}

		return 1;
	} else 	if (reportId == 4) {
		// switch color order "G,R,B"
		ledStatus[data[1]].g = data[3];
		ledStatus[data[1]].r = data[2];
		ledStatus[data[1]].b = data[4];

		cli(); //Disable interrupts
		ws2812_setleds(&ledStatus[0], sizeof(ledStatus));
		sei(); //Enable interrupts

		return 1;
	}else if (reportId == 2 || reportId == 3) {
		if(bytesRemaining == 0)
			return 1; // end of transfer 

		if(len > bytesRemaining)
			len = bytesRemaining;

		//Ignore the first byte of data as it's report id
		if (currentAddress == 0){
			eeprom_write_block(&data[1], (uchar *)0 + currentAddress + addressOffset, len);
			currentAddress += len - 1;
			bytesRemaining -= (len - 1);
		}else {
			eeprom_write_block(data, (uchar *)0 + currentAddress + addressOffset, len);
			currentAddress += len;
			bytesRemaining -= len;
		}

		return bytesRemaining == 0; // return 1 if this was the last chunk 
	} else if (reportId==5){
		display7sSet(data[1]);
		return 1;
	}
	else 
		return 1;
}

#define SERIAL_NUMBER_LENGTH 12 // BSXXXXXX-1.0
static int  serialNumberDescriptor[SERIAL_NUMBER_LENGTH + 1];

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
   eeprom_read_block(serialNumber, (uchar *)0 + 1, SERIAL_NUMBER_LENGTH);

   for (int i =0; i < SERIAL_NUMBER_LENGTH; i++)
	serialNumberDescriptor[i +1] = serialNumber[i];
}

/* ------------------------------------------------------------------------- */

extern "C" usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (usbRequest_t *)data;
	reportId = rq->wValue.bytes[0];

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){ /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){ /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			 usbMsgPtr = replyBuffer;

			 if(reportId == 1) { //Device colors
				 
				replyBuffer[0] = 1; //report id
				replyBuffer[1] = r;
				replyBuffer[2] = g;
				replyBuffer[3] = b;

				return 4;
			 } else if(reportId == 2){ // Name of the device
				 replyBuffer[0] = 2; //report id

				 bytesRemaining = 33;
				 currentAddress = 0;

				 addressOffset = 32;

				 return USB_NO_MSG; /* use usbFunctionRead() to obtain data */
			 } else if(reportId == 3){ // Name of the device
				 replyBuffer[0] = 3; //report id

				 bytesRemaining = 33;
				 currentAddress = 0;

				 addressOffset = 64;

				 return USB_NO_MSG; /* use usbFunctionRead() to obtain data */
			 }

			 return 0;

        } else if(rq->bRequest == USBRQ_HID_SET_REPORT){
			 if(reportId == 1){ // Device colors
				bytesRemaining = 3;
				currentAddress = 0;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
			 }
			 else if(reportId == 2){ // Name of the device
				currentAddress = 0;
				bytesRemaining = 32;

				addressOffset = 32;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
			 }
			 else if(reportId == 3){ // Name of the device
				currentAddress = 0;
				bytesRemaining = 32;

				addressOffset = 64;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
			 }
			 else if(reportId == 4){ // Set n LED to RGB color
				bytesRemaining = 4;
				currentAddress = 0;
				return USB_NO_MSG; /* use usbFunctionWrite() to receive data from host */
			 } else if (reportId == 5) {
				bytesRemaining = 1;
				currentAddress = 0 ;
				return USB_NO_MSG;
			 }
			 return 0;
        }
	}

    return 0;   /* default for not implemented requests: return no data back to host */
}

static void calibrateOscillator(void) {
	uchar       step = 128;
	uchar       trialValue = 0, optimumValue;
	int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
 
    /* do a binary search: */
    do{
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    // proportional to current real frequency
        if(x < targetValue)             // frequency still too low
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
    cli();  // usbMeasureFrameLength() counts CPU cycles, so disable interrupts.
    calibrateOscillator();
    sei();
    eeprom_write_byte(0, OSCCAL);   // store the calibrated value in EEPROM
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

    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    
	uchar i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
	
    usbDeviceConnect();

	// Initialize all leds to 0
	for (i = 0; i < NUMBER_OF_LEDS; i++)
		ledStatus[i] = { 0, 0, 0 };
  
	sei();

	blinkled();

   for(;;){                /* main event loop */
        wdt_reset();
        usbPoll();
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
