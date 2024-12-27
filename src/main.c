////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
// 01.07.2024 by magner
////////////////////////////////////////////////////////

// добавляем в platformio.ini - успокаиваем vscode intellisence
/* build_flags =
    -I ".\src\kernel\include"
    -D "__flash="
    -D "__memx=" */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <math.h>

/* RTOS Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

#include "board.h"
#include "modbus.h"
#include "totalizer.h"
#include "avr8gpio.h"

#include "lcd.h"
#include "char_pattern.h"
#include "onebutton.h"
#include "char_pattern.h"
#include "fmenu.h"
#include "fmenu_items.h"

#include "i2cMultiMaster.h"
#include "rtc.h"
#include "mcp401x.h"
#include "mcp4726.h"
#include "yaMBSiavr.h"

/* serial interface include file. */
#include "serial.h"
#include "ver.h"

#define REG_COUNT 16

// volatile uint8_t instate = 0;
volatile uint16_t inputRegisters[REG_COUNT];
volatile uint16_t holdingRegisters[REG_COUNT];

volatile uint16_t g_periodH;
volatile uint32_t g_period;
volatile uint32_t g_pulses0;
volatile uint32_t g_pulses1;
volatile uint32_t g_pulses2;
volatile uint32_t g_pulses3;

TimerHandle_t xTimerBeep;
TimerHandle_t xTimerPulse;

static void TaskPollButton(void *pvParameters);
static void TaskModbus(void *pvParameters);
static void boardInit(void);
static void deadBeef(void);

void prvBeepEnable(BaseType_t tone, uint16_t duration);
void prvBeepDisable(TimerHandle_t xTimer);
void prvPulseEnd(TimerHandle_t xTimer);

void meterScreen();

/* Main program loop */
int main(void) __attribute__((OS_main));

int main(void)
{
    // setup board specific hardware
    boardInit();
    // turn on the serial port for debugging or for other USART reasons.
    xSerialPort = xSerialPortInitMinimal(USART0, 115200, portSERIAL_BUFFER_TX, portSERIAL_BUFFER_RX); //  serial port: WantedBaud, TxQueueLength, RxQueueLength (8n1)
    avrSerialxPrint_P(&xSerial1Port, PSTR("SKE-02 with FreeRTOS - we alive!\n"));                     // Ok, so we're alive...

    lcd_init(LCD_DISP_ON);
    OBInit();
    modbusSetAddress(1);
    modbusInit();

    lcd_gotoxy(0, 0);
    lcd_puts_p(PSTR("Green Board\n"));
    lcd_puts_p(PSTR("FreeRTOS API\n"));
    lcd_puts_p(PSTR("Tick: 1ms\n"));
    lcd_Printf_P(PSTR("Free Heap Size: %u"), xPortGetFreeHeapSize());
    _delay_ms(1000);
    lcd_clrscr();

    inputRegisters[0] = 12345;
    inputRegisters[1] = 23456;
    inputRegisters[2] = 11111;
    inputRegisters[3] = 22222;

    /* create the event group. */
    // xFlagsEventGroup = xEventGroupCreate();

    //  create queue for 5 parameters
    // xQueueMeter = xQueueCreate(5, sizeof(float));
    // if (xQueueMeter == NULL)
    // deadBeef();
    xTimerBeep = xTimerCreate(PSTR("Beep"), pdMS_TO_TICKS(10), pdFALSE, 0, prvBeepDisable);
    xTimerPulse = xTimerCreate(PSTR("Pulse"), pdMS_TO_TICKS(10), pdFALSE, 0, prvPulseEnd);

    xTaskCreate(TaskPollButton, (const char *)"PollButton", 256, NULL, 2, NULL); // Tested 9 free @ 208
    // xTaskCreate(TaskMeter, (const char *)"Count", 256, NULL, 1, NULL);           // Tested 9 free @ 208
    xTaskCreate(TaskModbus, (const char *)"TaskModbus", 256, NULL, 1, NULL); // Tested 9 free @ 208

    vTaskStartScheduler();
    deadBeef();
}
/*-----------------------------------------------------------*/
static void TaskPollButton(void *pvParameters)
{
    uint8_t key_cancel;
    uint8_t key_enter;
    uint8_t key_down;
    uint8_t key_up;

    for (;;)
    {
        // GPTOGGLE(GPE5);
        key_cancel = OBtick(!GPREAD(GPC0), 0); // cancel
        key_enter = OBtick(!GPREAD(GPC1), 1);  // enter
        key_down = OBtick(!GPREAD(GPC2), 2);   // down
        key_up = OBtick(!GPREAD(GPC3), 3);     // up

        if (key_cancel == OB_LONGPRESSSTART)
        {
            prvBeepEnable(0x30, 50);
            g_pulses0 = 0;
            g_pulses1 = 0;
        }

        if (key_up == OB_CLICK)
        {
            prvBeepEnable(0x20, 20);
            GPSET(DOUT0);
            xTimerChangePeriod(xTimerPulse, pdMS_TO_TICKS(20), 0);
        }

        meterScreen();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void meterScreen()
{
    lcd_gotoxy(0, 0);
    lcd_Printf_P(PSTR("ch0%17u"), g_pulses0);
    lcd_gotoxy(0, 1);
    lcd_Printf_P(PSTR("ch1%17u"), g_pulses1);
    lcd_gotoxy(0, 2);
    lcd_Printf_P(PSTR("ch2%17u"), g_pulses2);
    lcd_gotoxy(0, 3);
    lcd_Printf_P(PSTR("ch3%17u"), g_pulses3);
}

static void TaskModbus(void *pvParameters)
{
    for (;;)
    {
        vTaskDelay(1);
        modbusTickTimer();
        if (modbusGetBusState() & (1 << ReceiveCompleted))
        {
            switch (rxbuffer[1])
            {
            case fcReadHoldingRegisters:
                modbusExchangeRegisters(holdingRegisters, 0, REG_COUNT);
                break;
            case fcReadInputRegisters:
                modbusExchangeRegisters(inputRegisters, 0, REG_COUNT);
                break;
            case fcPresetSingleRegister:
                modbusExchangeRegisters(holdingRegisters, 0, REG_COUNT);
                break;
            case fcPresetMultipleRegisters:
                modbusExchangeRegisters(holdingRegisters, 0, REG_COUNT);
                break;
            default:
                modbusSendException(ecIllegalFunction);
                break;
            }
        }
    }
}
/*-----------------------------------------------------------*/

// beep function
void prvBeepEnable(BaseType_t tone, uint16_t duration)
{
    xTimerChangePeriod(xTimerBeep, duration, 0);
    OCR0 = tone;
    TCCR0 |= (1 << COM00); // enable OC0 output
}
/*-----------------------------------------------------------*/

// timer callback
void prvBeepDisable(TimerHandle_t xTimer)
{
    TCCR0 &= ~(1 << COM00); // disable OC0 output
}
/*-----------------------------------------------------------*/
void prvPulseEnd(TimerHandle_t xTimer)
{
    GPCLEAR(DOUT0);
}
/*-----------------------------------------------------------*/

// Setup GPIO, timers, interrupts before RTOS
static void boardInit(void)
{
    // timer0 setup - BUZZZER chanell
    TCCR0 = _BV(WGM01) | _BV(CS01) | _BV(CS00); // CTC, prescaler 32

    // timer1 setup
    TCCR1A = (1 << WGM11) | (1 << COM1B1);
    TCCR1B = (1 << CS10) | (1 << CS11);

    OCR1A = 0x0100; // set PWM value - DOUT1
    OCR1B = 0x0140; // LCD Contrast set PWM value
    OCR1C = 0x0180; // set PWM value - DOUT0

    // timer2 setup as tick FreeRTOS in port.c
    // OCR2 = 249;
    // TCCR2 = _BV(COM20)| _BV(WGM21) | _BV(CS21) | _BV(CS20);

    // timer3 setup as counter with CLC/1 freq
    TCCR3B = _BV(CS30); // clc/1 no prescaling

    // GPIO setup
    DDRB = GPBV(BUZZER) | GPBV(LCD_CON) | GPBV(DOUT0) | GPBV(DOUT1);
    DDRC = 0; // all inputs
    DDRD = GPBV(DOUT2);
    DDRE |= GPBV(DLED) | GPBV(TP7);
    DDRG |= GPBV(LCD_BL) | GPBV(VMOTOR);
    DDRF = GPBV(SENSOR_MODE);

    PORTC = 0xff; // all pullup
    GPSET(LCD_BL);

    // Analog comparator configuration. Comparator Interrupt on falling edge
    ACSR = (1 << ACBG) | (1 << ACIE) | (1 << ACIS1); // Analog comparator configuration. Comparator Interrupt on falling edge

    // interrupts setup
    EICRB = _BV(ISC71);  // INT7 falling edge mode
    EIMSK = _BV(INT7);   // enable INT7 interrupt
    ETIMSK = _BV(TOIE3); // enable timer3 overflow interrupt

    // blink led - we alive
    for (uint8_t i = 0; i < 3; i++)
    {
        GPSET(DLED);
        _delay_ms(100);
        GPCLEAR(DLED);
        _delay_ms(300);
    }
    // PORTG |= _BV(LCD_BL); // LCD backlight enable
}
/*-----------------------------------------------------------*/

// sheduler start failure
static void deadBeef()
{
    // blink led - we dead
    for (;;)
    {
        PORTE |= _BV(PE5);
    }
}

// INT7 interrupt
ISR(INT7_vect)
{
    uint16_t tcnt16;
    tcnt16 = TCNT3;
    TCNT3 = 0;
    g_period = g_period + tcnt16 + g_periodH * 0x010000L;
    g_pulses0++;
    g_periodH = 0;
}
/*-----------------------------------------------------------*/

// timer3 overflow vector
ISR(TIMER3_OVF_vect)
{
    g_periodH++;
}
/*-----------------------------------------------------------*/

// AC routine
ISR(ANALOG_COMP_vect) // PE3 AIN1
{
    // GPTOGGLE(GPE5);
    g_pulses1++;
}
/*-----------------------------------------------------------*/
