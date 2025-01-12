/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef PORTMACRO_H
#define PORTMACRO_H

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C"
{
#endif
/* *INDENT-ON* */

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */

/* Type definitions. */
#define portCHAR char
#define portFLOAT float
#define portDOUBLE double
#define portLONG long
#define portSHORT int
#define portSTACK_TYPE uint8_t
#define portBASE_TYPE char

#define portPOINTER_SIZE_TYPE uint16_t

    typedef uint8_t StackType_t;
    typedef int8_t BaseType_t;
    typedef uint8_t UBaseType_t;

#if (configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_16_BITS)
    typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)(0xffffU)
#elif (configTICK_TYPE_WIDTH_IN_BITS == TICK_TYPE_WIDTH_32_BITS)
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)(0xffffffffUL)
#else
#error configTICK_TYPE_WIDTH_IN_BITS set to unsupported tick type width.
#endif
    /*-----------------------------------------------------------*/

    /* Critical section management. */

#define portENTER_CRITICAL()       \
    __asm__ __volatile__(          \
        "in __tmp_reg__, __SREG__" \
        "\n\t"                     \
        "cli"                      \
        "\n\t"                     \
        "push __tmp_reg__"         \
        "\n\t" ::: "memory")

#define portEXIT_CRITICAL()         \
    __asm__ __volatile__(           \
        "pop __tmp_reg__"           \
        "\n\t"                      \
        "out __SREG__, __tmp_reg__" \
        "\n\t" ::: "memory")

#define portDISABLE_INTERRUPTS() __asm__ __volatile__("cli" ::: "memory")
#define portENABLE_INTERRUPTS() __asm__ __volatile__("sei" ::: "memory")
    /*-----------------------------------------------------------*/

    /* Architecture specifics. */

#define sleep_reset()           \
    do                          \
    {                           \
        _SLEEP_CONTROL_REG = 0; \
    } while (0) /* reset all sleep_mode() configurations. */

#define portSTACK_GROWTH (-1)
#define portBYTE_ALIGNMENT 1
#define portNOP() __asm__ __volatile__("nop");
    /*-----------------------------------------------------------*/

    /* Kernel utilities. */

    extern void vPortYield(void) __attribute__((naked));
#define portYIELD() vPortYield()

    extern void vPortYieldFromISR(void) __attribute__((naked));
#define portYIELD_FROM_ISR() vPortYieldFromISR()
    /*-----------------------------------------------------------*/

#if defined(__AVR_3_BYTE_PC__)
    /* Task function macros as described on the FreeRTOS.org WEB site. */

    /* Add .lowtext tag from the avr linker script avr6.x for ATmega2560 and ATmega2561
     * to make sure functions are loaded in low memory.
     */
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void *pvParameters) __attribute__((section(".lowtext")))
#else
#define portTASK_FUNCTION_PROTO(vFunction, pvParameters) void vFunction(void *pvParameters)
#endif

#define portTASK_FUNCTION(vFunction, pvParameters) void vFunction(void *pvParameters)

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */

#endif /* PORTMACRO_H */
