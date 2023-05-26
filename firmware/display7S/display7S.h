#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */

static void display7sSet(unsigned char data){
	int i=0;
	int value=0;
	unsigned char oldpinstate=D7S_PIN;

	/* Set pin mode */
	D7S_PIN|=(1<<D7S_DATA)|(1<<D7S_SRCLK) | (1<<D7S_RCLK) ;

	for (i=0;i<8;i++){
		if (0x80 & (data << i))
			value=1;
		else
			value=0;

		// gpio_set_value(display_gpio[SDI_IDX], value);
		if (value)
			D7S_PORT|=(0x1 <<  D7S_DATA);
		else
			D7S_PORT&=~(1 <<  D7S_DATA);
		//gpio_set_value(display_gpio[SRCLK_IDX], 1);
		D7S_PORT|=	(1 <<  D7S_SRCLK);
		_delay_ms(2);
		//gpio_set_value(display_gpio[SRCLK_IDX], 0);
		D7S_PORT&=~(1 <<  D7S_SRCLK);
		_delay_ms(2);
	}

	//gpio_set_value(display_gpio[RCLK_IDX], 1);
	D7S_PORT|=	1 <<  D7S_RCLK;
	_delay_ms(2);
	//	gpio_set_value(display_gpio[RCLK_IDX], 0);
	D7S_PORT&=~(1 <<  D7S_RCLK);

	/* Restore pin mode */
	D7S_PIN=oldpinstate;
}
