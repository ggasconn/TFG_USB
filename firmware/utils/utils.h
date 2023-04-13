#ifndef UTILS_H_
#define UTILS_H_

#include <avr/io.h>         /* for DDRB and PORTB registers */
#include <util/delay.h>     /* for _delay_ms() */

#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

void blinkled(void);
void blinkledRx(void);
void display7sSet(unsigned char data);

#endif /* UTILS_H_ */