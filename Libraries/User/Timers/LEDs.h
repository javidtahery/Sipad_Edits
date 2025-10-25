#ifndef __LEDS_H_
#define __LEDS_H_

#include "main.h"


/* ................ Defines ................*/
#define LED_FULL_OFF                            0
#define LED_STEADY                              125             //fix for 5 seconds and pulse for 80 ms
#define LED_ON_80MS                             2
#define LED_ON_250MS                            6
#define LED_ON_500MS                            12
#define LED_ON_5000MS                           125
#define LED_OFF_80MS                            2
#define LED_OFF_250MS                           6
#define LED_OFF_1000MS                          25
#define LED_OFF_3000MS                          75
#define LED_OFF_5000MS                          125


/* ................ Enums ................*/
enum{
  LED_DEVICE_SLEEP              = 0,
  LED_DEVICE_NORMAL,            // 1
  LED_DEVICE_FOTA,              // 2
};

enum{
  GPS_LED                       = 0,
  SYS_LED,                      // 1
  GSM_LED,                      // 2
};

enum{
  LED_OFF                       = 0,
  LED_ON_STEADY,                // 1
  LED_ON_BLINK_500MS,           // 2
  LED_ON_PULSE_EACH_250MS,      // 3
  LED_ON_PULSE_EACH_1SEC,       // 4
  LED_ON_PULSE_EACH_3SEC,       // 5
  LED_ON_PULSE_EACH_5SEC,       // 6
  LED_ON_TWICE_EACH_1SEC,       // 7
  LED_ON_TIKTOK_250MS,          // 8
  LED_ON_PULSE_EACH_80MS,       // 9
  LED_OFF_PULSE_EACH_5SEC,      // 10
};


extern uint8_t gsm_led_pulse_mode;
extern uint8_t gps_led_pulse_mode;
extern uint8_t sys_led_pulse_mode;
extern uint8_t gsm_led_pulse_count;
extern uint8_t gps_led_pulse_count;
extern uint8_t sys_led_pulse_count;
extern uint8_t disable_accel_timer;
extern uint8_t sys_led_state;
extern uint8_t gsm_led_state;
extern uint8_t gps_led_state;


void LED_change_mode(uint8_t led, uint8_t led_mode);
void LED_Control_LEDs(void);


#endif