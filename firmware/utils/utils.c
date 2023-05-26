#include "utils.h"

#if PWM == 1
void blinkPWM(void) {
	DDRB   |= (1 << PWM_LED);      //Set PB1 as output
	TCCR1A |= (1 << COM1A0);      //Set PB1/OC1A to toggle on timer
	TCCR1B |= (1 << WGM12);       //Set timer CTC mode
	OCR1A   = 31249;              //Set count limit 1 sec @ 16MHz, 256 divisor
	TCCR1B |= (1 << CS12);
}

void hardwarePWMBeep(uint16_t frequency) {
	DDRB   |= (1 << BUZZER_PIN); //Set PB1 as output
	TCCR1A |= (1 << COM1A0); //Set PB1/OC1A to toggle on timer
	TCCR1B |= (1 << WGM12); //Set timer CTC mode
	OCR1A   = F_CPU / 2 / 1 - frequency; //Set count limit 1 sec @ 16MHz, 256 divisor
	TCCR1B |= (1 << CS10);
}
#endif

#if DISPLAYS == 1
void display7sSet(unsigned char data) {
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
#endif

void blinkled(void) {
	unsigned char oldstate;
	int i;

	oldstate=DDRB;
	DDRB |= (0x1 << BUILTIN_LED);

	/* Led test */
	for (i=0;i<3;i++) {
		PORTB |=(0x1 <<  BUILTIN_LED);
		_delay_ms(1000);
		PORTB &= ~(0x1 <<  BUILTIN_LED);
		_delay_ms(1000);
	}

	DDRB=oldstate;
}

void blinkledRx(void) {
	unsigned char oldstate;
	int i;

	oldstate=DDRD;
	DDRD |= (0x1 << RX_LED);

	/* Led test */
	for (i=0;i<3;i++) {
		PORTD|=(0x1 <<  RX_LED);
		_delay_ms(100);
		PORTD &= ~(0x1 <<  RX_LED);
		_delay_ms(100);
	}

	DDRD=oldstate;
}

void clearTimer1() {
	// Clear the CS12, CS11, and CS10 bits to disable Timer1
	TCCR1B &= ~(1 << CS12);
	TCCR1B &= ~(1 << CS11);
	TCCR1B &= ~(1 << CS10);

	// Clear the COM1A1 and COM1A0 bits to disable the PWM output on pin 9
	TCCR1A &= ~(1 << COM1A1);
	TCCR1A &= ~(1 << COM1A0);
}