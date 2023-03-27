#include "utils.h"
#include <avr/io.h>         /* for DDRB and PORTB registers */
#include <util/delay.h>     /* for _delay_ms() */

void blinkled(void) {
	unsigned char oldstate;
	int i;

	oldstate=DDRB;
	DDRB |= (0x1 << PB5);

	/* Led test */
	for (i=0;i<3;i++) {
		PORTB|=(0x1 <<  PB5);
		_delay_ms(100);
		PORTB &= ~(0x1 <<  PB5);
		_delay_ms(100);
	}
	DDRB=oldstate;
}

void blinkledRx(void) {
	unsigned char oldstate;
	int i;

	oldstate=DDRD;
	DDRD |= (0x1 << PD0);

	/* Led test */
	for (i=0;i<3;i++) {
		PORTD|=(0x1 <<  PD0);
		_delay_ms(100);
		PORTD &= ~(0x1 <<  PD0);
		_delay_ms(100);
	}
	DDRD=oldstate;
}

void display7sSet(unsigned char data) {
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
