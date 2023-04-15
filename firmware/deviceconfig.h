#include <avr/io.h>
#include <avr/pgmspace.h>       // for using data in program space

// change values for attiny85
#ifdef attiny45
#define DEVICE_PORT PORTB
#define DEVICE_PIN DDRB
 
//OLED Device 
#define OLED_SCL PB2
#define OLED_SDA PB0

// LED ring
#define LED_RING_PIN 0
#define LED_RING_PORT B

// Display 7seg
#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

#elif defined(atmega328p)
#define DEVICE_PORT PORTB
#define DEVICE_PIN DDRB
 
//OLED Device 
#define OLED_SCL PB2
#define OLED_SDA PB0

// LED ring
#define LED_RING_PIN 0
#define LED_RING_PORT B

// Display 7seg
#define	D7S_DATA 0
#define	D7S_RCLK 1
#define	D7S_SRCLK 2

#endif
