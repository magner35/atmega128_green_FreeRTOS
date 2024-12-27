
#ifndef _TOTALIZER_H_
#define _TOTALIZER_H_

/* RTOS Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

enum
{
    DMODE_MAIN,
    DMODE_SETUP,
    DMODE_DEBUG
};

#define EV_TOTALRESET (1 << 0)
#define EV_GTOTALRESET (1 << 1)
#define EV_PASSTIMEOUT (1 << 7)


void prvBeepEnable(BaseType_t tone, uint16_t duration);

void prvMotorEnable(uint16_t duration);

void prvBeepDisable(TimerHandle_t xTimer);

#endif
