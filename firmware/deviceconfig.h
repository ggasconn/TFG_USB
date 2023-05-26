/****************************/
/*        TEMPLATE          */
/****************************/

#define ON  1
#define OFF  0

#define ALL_DEVICES  ON
#define LED_PARTY  OFF || ALL_DEVICES
#define DISPLAYS  OFF || ALL_DEVICES
#define PWM  OFF || ALL_DEVICES

#if LED_PARTY == 1
    // LED ring
    #define LED_RING_PORT B
    #define LED_RING_PIN 0
    #define BUILTIN_LED PB5
    #define RX_LED PD0
#endif

#if DISPLAYS == 1
    // OLED
    #define OLED_PORT PORTB
    #define OLED_PIN DDRB
    #define OLED_SCL PB2
    #define OLED_SDA PB0

    // Display 7seg
    #define D7S_PORT PORTD
    #define D7S_PIN DDRD
    #define	D7S_DATA 5
    #define	D7S_RCLK 6
    #define	D7S_SRCLK 7
#endif

#if PWM == 1
    #define BUZZER_PIN PORTB1
    #define PWM_LED PORTB1
#endif
