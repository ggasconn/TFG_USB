#ifndef UTILS_H_
#define UTILS_H_

#include <avr/io.h>         /* for DDRB and PORTB registers */
#include <util/delay.h>

#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

#define BUZZER_PIN PB0 // set the pin for the buzzer

void blinkled(void);
void blinkPWM(void);
void blinkledRx(void);
void display7sSet(unsigned char data);
void hardwarePWMBeep(uint16_t frequency);
void clearTimer1();

#endif /* UTILS_H_ */