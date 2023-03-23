#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */

#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

static void display7sSet(unsigned char data){
	int i=0;
	int value=0;
	unsigned char oldpinstate=DDRB;

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
		_delay_ms(2);
		//gpio_set_value(display_gpio[SRCLK_IDX], 0);
		PORTB&=~(1 <<  D7S_SRCLK);
		_delay_ms(2);
	}

	//gpio_set_value(display_gpio[RCLK_IDX], 1);
	PORTB|=	1 <<  D7S_RCLK;
	_delay_ms(2);
	//	gpio_set_value(display_gpio[RCLK_IDX], 0);
	PORTB&=~(1 <<  D7S_RCLK);

	/* Restore pin mode */
	DDRB=oldpinstate;
}