/****************************************************************************
 Title	:   HD44780U LCD library
 Author:    Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 File:	    $Id: lcd.c,v 1.14.2.2 2012/02/12 07:51:00 peter Exp $
 Software:  AVR-GCC 3.3
 Target:    any AVR device, memory mapped mode only for AT90S4414/8515/Mega

 DESCRIPTION
       Basic routines for interfacing a HD44780U-based text lcd display

       Originally based on Volker Oth's lcd library,
       changed lcd_init(), added additional constants for lcd_command(),
       added 4-bit I/O mode, improved and optimized code.

       Library can be operated in memory mapped mode (LCD_IO_MODE=0) or in
       4-bit IO port mode (LCD_IO_MODE=1). 8-bit IO port mode not supported.

       Memory mapped mode compatible with Kanda STK200, but supports also
       generation of R/W signal through A8 address line.

 USAGE
       See the C include lcd.h file for a description of each function

*****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "char_pattern.h"

#include "FreeRTOS.h"

char *LCDWorkBuffer; // create a working buffer pointer, to later be pvPortMalloc() on the heap.

/*
** constants/macros
*/
#define DDR(x) (*(&x - 1)) /* address of data direction register of port x */
#if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
/* on ATmega64/128 PINF is on port 0x00 and not 0x60 */
#define PIN(x) (&PORTF == &(x) ? _SFR_IO8(0x00) : (*(&x - 2)))
#else
#define PIN(x) (*(&x - 2)) /* address of input register of port x          */
#endif

#if LCD_IO_MODE
// #define lcd_e_delay()   __asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_delay() __asm__ __volatile__("rjmp 1f\n 1: rjmp 2f\n 2:");
#define lcd_e_high() LCD_E_PORT |= _BV(LCD_E_PIN);
#define lcd_e_low() LCD_E_PORT &= ~_BV(LCD_E_PIN);
#define lcd_e_toggle() toggle_e()
#define lcd_rw_high() LCD_RW_PORT |= _BV(LCD_RW_PIN)
#define lcd_rw_low() LCD_RW_PORT &= ~_BV(LCD_RW_PIN)
#define lcd_rs_high() LCD_RS_PORT |= _BV(LCD_RS_PIN)
#define lcd_rs_low() LCD_RS_PORT &= ~_BV(LCD_RS_PIN)
#endif

#if LCD_LINES == 1
#define LCD_FUNCTION_DEFAULT LCD_FUNCTION_4BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT LCD_FUNCTION_4BIT_2LINES_RUS
#endif

/*
** function prototypes
*/
#if LCD_IO_MODE
static void toggle_e(void);
#endif

/*
** local functions
*/

/*************************************************************************
 delay loop for small accurate delays: 16-bit counter, 4 cycles/loop
*************************************************************************/
static inline void _delayFourCycles(unsigned int __count)
{
    if (__count == 0)
        __asm__ __volatile__("rjmp 1f\n 1:"); // 2 cycles
    else
        __asm__ __volatile__(
            "1: sbiw %0,1"
            "\n\t"
            "brne 1b" // 4 cycles/loop
            : "=w"(__count)
            : "0"(__count));
}

/*************************************************************************
delay for a minimum of <us> microseconds
the number of loops is calculated at compile-time from MCU clock frequency
*************************************************************************/
#define delay(us) _delayFourCycles(((1 * (XTAL / 4000)) * us) / 1000)

#if LCD_IO_MODE
/* toggle Enable Pin to initiate write */
static void toggle_e(void)
{
    lcd_e_high();
    lcd_e_delay();
    lcd_e_low();
}
#endif

/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data, uint8_t rs)
{
    unsigned char dataBits;
    if (rs)
    { /* write data        (RS=1, RW=0) */
        lcd_rs_high();
    }
    else
    { /* write instruction (RS=0, RW=0) */
        lcd_rs_low();
    }
    lcd_rw_low();

    if ((&LCD_DATA0_PORT == &LCD_DATA1_PORT) && (&LCD_DATA1_PORT == &LCD_DATA2_PORT) && (&LCD_DATA2_PORT == &LCD_DATA3_PORT) && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3))
    {
        /* configure data pins as output */
        DDR(LCD_DATA0_PORT) |= 0x0F;
        /* output high nibble first */
        dataBits = LCD_DATA0_PORT & 0xF0;
        LCD_DATA0_PORT = dataBits | ((data >> 4) & 0x0F);
        lcd_e_toggle();
        /* output low nibble */
        LCD_DATA0_PORT = dataBits | (data & 0x0F);
        lcd_e_toggle();
        /* all data pins high (inactive) */
        LCD_DATA0_PORT = dataBits | 0x0F;
    }
    else
    {
        /* configure data pins as output */
        DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
        /* output high nibble first */
        LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
        LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
        LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
        LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
        if (data & 0x80)
            LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
        if (data & 0x40)
            LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
        if (data & 0x20)
            LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
        if (data & 0x10)
            LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
        lcd_e_toggle();
        /* output low nibble */
        LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
        LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
        LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
        LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);
        if (data & 0x08)
            LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
        if (data & 0x04)
            LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
        if (data & 0x02)
            LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
        if (data & 0x01)
            LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
        lcd_e_toggle();
        /* all data pins high (inactive) */
        LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
        LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
        LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
        LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
    }
}

/*************************************************************************
Low-level function to read byte from LCD controller
Input:    rs     1: read data
                 0: read busy flag / address counter
Returns:  byte read from LCD controller
*************************************************************************/
static uint8_t lcd_read(uint8_t rs)
{
    uint8_t data;
    if (rs)
        lcd_rs_high(); /* RS=1: read data      */
    else
        lcd_rs_low(); /* RS=0: read busy flag */
    lcd_rw_high();    /* RW=1  read mode      */

    if ((&LCD_DATA0_PORT == &LCD_DATA1_PORT) && (&LCD_DATA1_PORT == &LCD_DATA2_PORT) && (&LCD_DATA2_PORT == &LCD_DATA3_PORT) && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3))
    {
        DDR(LCD_DATA0_PORT) &= 0xF0; /* configure data pins as input */
        lcd_e_high();
        lcd_e_delay();
        data = PIN(LCD_DATA0_PORT) << 4; /* read high nibble first */
        lcd_e_low();
        lcd_e_delay(); /* Enable 500ns low       */
        lcd_e_high();
        lcd_e_delay();
        data |= PIN(LCD_DATA0_PORT) & 0x0F; /* read low nibble        */
        lcd_e_low();
    }
    else
    {
        /* configure data pins as input */
        DDR(LCD_DATA0_PORT) &= ~_BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) &= ~_BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) &= ~_BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) &= ~_BV(LCD_DATA3_PIN);
        /* read high nibble first */
        lcd_e_high();
        lcd_e_delay();
        data = 0;
        if (PIN(LCD_DATA0_PORT) & _BV(LCD_DATA0_PIN))
            data |= 0x10;
        if (PIN(LCD_DATA1_PORT) & _BV(LCD_DATA1_PIN))
            data |= 0x20;
        if (PIN(LCD_DATA2_PORT) & _BV(LCD_DATA2_PIN))
            data |= 0x40;
        if (PIN(LCD_DATA3_PORT) & _BV(LCD_DATA3_PIN))
            data |= 0x80;
        lcd_e_low();
        lcd_e_delay(); /* Enable 500ns low       */
        /* read low nibble */
        lcd_e_high();
        lcd_e_delay();
        if (PIN(LCD_DATA0_PORT) & _BV(LCD_DATA0_PIN))
            data |= 0x01;
        if (PIN(LCD_DATA1_PORT) & _BV(LCD_DATA1_PIN))
            data |= 0x02;
        if (PIN(LCD_DATA2_PORT) & _BV(LCD_DATA2_PIN))
            data |= 0x04;
        if (PIN(LCD_DATA3_PORT) & _BV(LCD_DATA3_PIN))
            data |= 0x08;
        lcd_e_low();
    }
    return data;
}

/*************************************************************************
loops while lcd is busy, returns address counter
*************************************************************************/
static uint8_t lcd_waitbusy(void)
{
    register uint8_t c;
    /* wait until busy flag is cleared */
    while ((c = lcd_read(0)) & (1 << LCD_BUSY))
    {
    }

    /* the address counter is updated 4us after the busy flag is cleared */
    delay(4);
    /* now read the address counter */
    return (lcd_read(0)); // return address counter
} /* lcd_waitbusy */

/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor
is already on the last line.
*************************************************************************/
static inline void lcd_newline(uint8_t pos)
{
    register uint8_t addressCounter;
#if LCD_LINES == 1
    addressCounter = 0;
#endif
#if LCD_LINES == 2
    if (pos < (LCD_START_LINE2))
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES == 4
#ifdef KS0073_4LINES_MODE
    if (pos < LCD_START_LINE2)
        addressCounter = LCD_START_LINE2;
    else if ((pos >= LCD_START_LINE2) && (pos < LCD_START_LINE3))
        addressCounter = LCD_START_LINE3;
    else if ((pos >= LCD_START_LINE3) && (pos < LCD_START_LINE4))
        addressCounter = LCD_START_LINE4;
    else
        addressCounter = LCD_START_LINE1;
#else
    if (pos < LCD_START_LINE3)
        addressCounter = LCD_START_LINE2;
    else if ((pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4))
        addressCounter = LCD_START_LINE3;
    else if ((pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2))
        addressCounter = LCD_START_LINE4;
    else
        addressCounter = LCD_START_LINE1;
#endif
#endif
    lcd_command((1 << LCD_DDRAM) + addressCounter);
} /* lcd_newline */

/*
** PUBLIC FUNCTIONS
*/

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(uint8_t cmd)
{
    lcd_waitbusy();
    lcd_write(cmd, 0);
}

/*************************************************************************
Send data byte to LCD controller
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(uint8_t data)
{
    lcd_waitbusy();
    lcd_write(data, 1);
}

/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(uint8_t x, uint8_t y)
{
#if LCD_LINES == 1
    lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
#endif
#if LCD_LINES == 2
    if (y == 0)
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
    else
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE2 + x);
#endif
#if LCD_LINES == 4
    if (y == 0)
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
    else if (y == 1)
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE2 + x);
    else if (y == 2)
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE3 + x);
    else /* y==3 */
        lcd_command((1 << LCD_DDRAM) + LCD_START_LINE4 + x);
#endif

} /* lcd_gotoxy */

/*************************************************************************
*************************************************************************/
int lcd_getxy(void)
{
    return lcd_waitbusy();
}

/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1 << LCD_CLR);
    lcd_gotoxy(0, 0);
}

/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(void)
{
    lcd_command(1 << LCD_HOME);
}

/*************************************************************************
Display character at current cursor position
Input:    character to be displayed
Returns:  none
*************************************************************************/
void lcd_putc(char c)
{
    uint8_t pos;
    pos = lcd_waitbusy(); // read busy-flag and address counter
    if (c == '\n')
    {
        lcd_newline(pos);
    }
    else
    {
#if LCD_WRAP_LINES == 1
#if LCD_LINES == 1
        if (pos == LCD_START_LINE1 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE1, 0);
        }
#elif LCD_LINES == 2
        if (pos == LCD_START_LINE1 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE2, 0);
        }
        else if (pos == LCD_START_LINE2 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE1, 0);
        }
#elif LCD_LINES == 4
        if (pos == LCD_START_LINE1 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE2, 0);
        }
        else if (pos == LCD_START_LINE2 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE3, 0);
        }
        else if (pos == LCD_START_LINE3 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE4, 0);
        }
        else if (pos == LCD_START_LINE4 + LCD_DISP_LENGTH)
        {
            lcd_write((1 << LCD_DDRAM) + LCD_START_LINE1, 0);
        }
#endif
        lcd_waitbusy();
#endif
        lcd_write(c, 1);
    }

} /* lcd_putc */

/*************************************************************************
Display string without auto linefeed
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(char *str)
/* print string on lcd (no auto linefeed) */
{
    int16_t i = 0;
    size_t stringlength;
    if (strlen((char *)str) < portLCD_BUFFER)
        stringlength = strlen((char *)str);
    else
        stringlength = portLCD_BUFFER - 1;
    while (i < stringlength)
        lcd_putc(str[i++]);
} /* lcd_puts */

/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;
    while ((c = pgm_read_byte(progmem_s++)))
    {
        lcd_putc(c);
    }
} /* lcd_puts_p */

/*************************************************************************
Initialize display and select type of cursor
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init(uint8_t dispAttr)
{
    // create a working buffer for vsnprintf on the heap (so we can use extended RAM, if available).
    if (LCDWorkBuffer == NULL) // if there is no LCDWorkBuffer allocated (pointer is NULL), then allocate buffer.
        if (!(LCDWorkBuffer = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * portLCD_BUFFER)))
            return;

    /*
     *  Initialize LCD to 4 bit I/O mode
     */

    if ((&LCD_DATA0_PORT == &LCD_DATA1_PORT) && (&LCD_DATA1_PORT == &LCD_DATA2_PORT) && (&LCD_DATA2_PORT == &LCD_DATA3_PORT) && (&LCD_RS_PORT == &LCD_DATA0_PORT) && (&LCD_RW_PORT == &LCD_DATA0_PORT) && (&LCD_E_PORT == &LCD_DATA0_PORT) && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) && (LCD_RS_PIN == 4) && (LCD_RW_PIN == 5) && (LCD_E_PIN == 6))
    {
        /* configure all port bits as output (all LCD lines on same port) */
        DDR(LCD_DATA0_PORT) |= 0x7F;
    }
    else if ((&LCD_DATA0_PORT == &LCD_DATA1_PORT) && (&LCD_DATA1_PORT == &LCD_DATA2_PORT) && (&LCD_DATA2_PORT == &LCD_DATA3_PORT) && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3))
    {
        /* configure all port bits as output (all LCD data lines on same port, but control lines on different ports) */
        DDR(LCD_DATA0_PORT) |= 0x0F;
        DDR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
        DDR(LCD_RW_PORT) |= _BV(LCD_RW_PIN);
        DDR(LCD_E_PORT) |= _BV(LCD_E_PIN);
    }
    else
    {
        /* configure all port bits as output (LCD data and control lines on different ports */
        DDR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
        DDR(LCD_RW_PORT) |= _BV(LCD_RW_PIN);
        DDR(LCD_E_PORT) |= _BV(LCD_E_PIN);
        DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
    }
    delay(32000); /* wait 16ms or more after power-on       */

    /* initial write to lcd is 8bit */
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN); // _BV(LCD_FUNCTION)>>4;
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN); // _BV(LCD_FUNCTION_8BIT)>>4;
    lcd_e_toggle();
    delay(4992); /* delay, busy flag can't be checked here */

    /* repeat last command */
    lcd_e_toggle();
    delay(64); /* delay, busy flag can't be checked here */

    /* repeat last command a third time */
    lcd_e_toggle();
    delay(64); /* delay, busy flag can't be checked here */

    /* now configure for 4bit mode */
    LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN); // LCD_FUNCTION_4BIT_1LINE>>4
    lcd_e_toggle();
    delay(64); /* some displays need this additional delay */

    /* from now the LCD only accepts 4 bit I/O, we can use lcd_command() */
    lcd_command(LCD_FUNCTION_DEFAULT); /* function set: display lines  */
    lcd_command(LCD_DISP_OFF);         /* display off                  */
    lcd_clrscr();                      /* display clear                */
    lcd_command(LCD_MODE_DEFAULT);     /* set entry mode               */
    lcd_command(dispAttr);             /* display/cursor control       */
} /* lcd_init */

/*-----------------------------------------------------------------------*/
/* Formatted Print Commands                                              */
/*-----------------------------------------------------------------------*/

// e.g. lcd_Printf_P(PSTR("\fMessage %u\r\n%u %u"), var1, var2, var2);

void lcd_Printf(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vsnprintf((char *)LCDWorkBuffer, portLCD_BUFFER, (const char *)format, arg);
    lcd_puts((uint8_t *)LCDWorkBuffer);
    va_end(arg);
}

void lcd_Printf_P(PGM_P format, ...)
{
    va_list arg;
    va_start(arg, format);
    vsnprintf_P((char *)LCDWorkBuffer, portLCD_BUFFER, format, arg);
    lcd_puts((uint8_t *)LCDWorkBuffer);
    va_end(arg);
}

// Create user character pattern
void lcd_createChar(
    uint8_t chr,    // Character code to be registered (0..7)
    _mem uint8_t *p // Pointer to the character pattern (8 * n bytes) in program memory
)
{
    lcd_command(_BV(LCD_CGRAM) + chr * 8); // set CG RAM start address
    for (uint8_t i = 0; i < 8; i++)
    {
        lcd_data(p[i]);
    }
}

#if (0)
_mem uint8_t bar_pattern_new[6][8] = {{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
                                      {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x15},
                                      {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1D},
                                      {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1D},
                                      {0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F},
                                      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15}};
#else
_mem uint8_t bar_pattern_new[6][8] = {{0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},
                                      {0x15, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x15},
                                      {0x1D, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1D},
                                      {0x1D, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1D},
                                      {0x1F, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1F},
                                      {0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15}};
#endif
/*-----------------------------------------------------------------------*/
/* Draw bargraph                                                         */
/*-----------------------------------------------------------------------*/
void lcd_PutBar(uint8_t val,  /* Bar length (0 to _MAX_BAR represents bar length from left end) */
                uint8_t width /* Display area (number of chars from cursor position) */
                              // uint8_t chr    /* User character code (2 chars used from this) */
)
{
    uint8_t i, gi, m;
    m = val % 5;
    gi = val / 5 + (m != 0); // round up
    i = lcd_waitbusy();
    lcd_createChar(6, *bar_pattern_new + m * 8);
    lcd_command(_BV(LCD_DDRAM) + i);
    for (i = 1; i <= width; i++)
    {
        if (i == gi)
            m = 6;
        else if (i > gi)
            m = 7;
        else
            m = 0xff;
        lcd_putc(m);
    }
}
