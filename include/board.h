#ifndef _BOARD_H_
#define _BOARD_H_

#include "avr8gpio.h"

#define DLED GPE5    // out PE5_(OC3C/INT5) Onboard debug green LED - active HIGH
#define LCD_BL GPG2  // out PG2_(ALE) LCD backlight - active HIGH
#define LCD_CON GPB6 // out PB6_(OC1B) LCD PWM contrast - active HIGH
#define BUZZER GPB4  // out PB4_(OC0) Buzzer PWM - active HIGH
#define ALED GPD7    // out PD7_(T2) ws2812 addresable LED
#define VMOTOR GPG0  // out PG0 Vibration Motor - active HIGH

#define DOUT0 GPB7  // out PB7_(OC2/OC1C) - active LOW
#define DOUT1 GPB5  // out PB_(OC1A) - active LOW
#define DOUT2 GPD4  // out PD4_(ICP1) - active LOW
#define DI2_EN GPE2 // out PE2_(XCK0/AIN0) - active LOW
#define DI2 GPE4    // in PE4_(OC3B/INT4) - active LOW

#define SENSOR_MODE GPF0 // out PF0_(ADC0) - coil - HIGH, reed - LOW

#define COUNT1 GPE7 // in PE7_(ICP3/INT7) - active LOW
#define COUNT2 GPE3 // in PE3_(OC3A/AIN1) - active LOW
#define TP7 GPE6    // test point 7 - PE6_(T3/INT6)
#define IDENT GPF2  // test point 44 - PF2_(ADC2)
#define TP44 IDENT

#define SW0 GPC0 // cancel
#define SW1 GPC1 // enter
#define SW2 GPC2 // down
#define SW3 GPC3 // up

#define INT_SW0 GPC4
#define INT_SW1 GPC5
#define INT_SW2 GPC6
#define INT_SW3 GPC7

#endif // _BOARD_H_