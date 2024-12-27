/* Is it safe to access global variables that are used by both ISRs and tasks by simply enclosing the access to the variable(s)
with enter / exit critical calls ? (see method 1)Or is the proper way to use the method found in tasks.c to get the TickCount
 ? (like method 2)(Using a PIC – ask me if i’m lovin it – NOT)Thanks !Ron.-———– Method 1 ————- interrrupts.c  */
volatile unsigned char ucPeriodDivider = 1;
#pragma interruptlow vIRQfunction save = PRODH, PRODL, section(".tmpdata") void vIRQfunction(void)
{
    ….OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_EDGE_RISE & ucPeriodDivider);
    ….
}
monitor.c extern volatile unsigned char ucPeriodDivider;
void vChangeThePeriod(void)
{
    taskENTER_CRITICAL();
    ucPeriodDivider = 3;
    taskEXIT_CRITICAL();
}
-————- Method 2 ————- interrrupts.c static volatile unsigned char ucPeriodDivider = 1;
#pragma interruptlow vIRQfunction save = PRODH, PRODL, section(".tmpdata") void vIRQfunction(void)
{
    ….OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_EDGE_RISE & ucPeriodDivider);
    ….
}
void vChangeThePeriod(volatile unsigned char ucPeriod)
{
    taskENTER_CRITICAL();
    ucPeriodDivider = ucPeriod;
    taskEXIT_CRITICAL();
}
monitor.c extern volatile unsigned char ucPeriodDivider;
void vMonitor(void)
{
    ….vChangeThePeriod(3);
    ….
}
Accessing variables from ISRs





/* Alternative: lock-free atomic reads via a repeat read loop: doAtomicRead(): ensure atomic reads withOUT turning interrupts off!
An alternative to using atomic access guards, as shown above, is to read the variable repeatedly until it doesn't change, indicating
 that the variable was not updated mid-read after you read only some bytes of it.
Note that this works on memory chunks of any size. The uint64_t type used in my examples below could instead be a struct my_struct of dozens
 or hundreds of bytes even. It is not limited to any size. doAtomicRead() still works.
Here is that approach. @Brendan and @chux-ReinstateMonica and I discussed some ideas of it under @chux-ReinstateMonica's answer. */

#include <stdint.h> // UINT64_MAX

#define MAX_NUM_ATOMIC_READ_ATTEMPTS 3

// errors
#define ATOMIC_READ_FAILED (UINT64_MAX)

    /// @brief          Use a repeat-read loop to do atomic-access reads of a
    ///     volatile variable, rather than using atomic access guards which
    ///     disable interrupts.
    uint64_t
    doAtomicRead(const volatile uint64_t *val)
{
    uint64_t val_copy;
    uint64_t val_copy_atomic = ATOMIC_READ_FAILED;

    for (size_t i = 0; i < MAX_NUM_ATOMIC_READ_ATTEMPTS; i++)
    {
        val_copy = *val;
        if (val_copy == *val)
        {
            val_copy_atomic = val_copy;
            break;
        }
    }

    return val_copy_atomic;
}
