#ifndef UTILS_H_
#define UTILS_H_

#include <avr/io.h>         /* for DDRB and PORTB registers */
#include <util/delay.h>     /* for _delay_ms() */
#include "../deviceconfig.h"

void blinkled(void);
void blinkledRx(void);
void display7sSet(unsigned char data);

#endif /* UTILS_H_ */
