#include <avr/io.h>
#include <util/delay.h>     /* for _delay_ms() */

static inline void blinkled(void){
	unsigned char oldstate;
	int i;
	oldstate=DDRB;
	DDRB|=(1<<PB1);

	/* Led test */
	for (i=0;i<3;i++){
		PORTB|=(0x1 <<  PB1);
		_delay_ms(100);
		PORTB&=~(1 <<  PB1);
		_delay_ms(100);
	}
	DDRB=oldstate;
}