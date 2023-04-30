#ifndef UTILS_H_
#define UTILS_H_

#include <avr/io.h>         /* for DDRB and PORTB registers */
#include <util/delay.h>     /* for _delay_ms() */

#if DISPLAYS == 1
void display7sSet(unsigned char data);
#endif

#if PWM == 1
void blinkPWM(void);
void hardwarePWMBeep(uint16_t frequency);
#endif

void blinkled(void);
void blinkledRx(void);
void clearTimer1();

#endif /* UTILS_H_ */
