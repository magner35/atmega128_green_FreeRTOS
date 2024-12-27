
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "cmsis_os.h"

/**
 * @brief Get a Message or Wait for a Message from a Queue.
 * @param  queue_id  message queue ID obtained with \ref osMessageCreate.
 * @param  millisec  timeout value or 0 in case of no time-out.
 * @retval event information that includes status code.
 * @note   MUST REMAIN UNCHANGED: \b osMessageGet shall be consistent in every CMSIS-RTOS.
 */
osEvent osMessageGet(QueueHandle_t queue_id, uint32_t millisec)
{
    TickType_t ticks;
    osEvent event;

    event.def.message_id = queue_id;
    event.value.v = 0;
    if (queue_id == NULL)
    {
        event.status = osErrorParameter;
        return event;
    }
    ticks = 0;
    if (millisec == osWaitForever)
    {
        ticks = portMAX_DELAY;
    }
    else if (millisec != 0)
    {
        ticks = millisec / portTICK_PERIOD_MS;
        if (ticks == 0)
        {
            ticks = 1;
        }
    }
    if (xQueueReceive(queue_id, &event.value.v, ticks) == pdTRUE)
    {
        /* We have mail */
        event.status = osEventMessage;
    }
    else
    {
        event.status = (ticks == 0) ? osOK : osEventTimeout;
    }
    return event;
}

/**
 * @brief Put a Message to a Queue.
 * @param  queue_id  message queue ID obtained with \ref osMessageCreate.
 * @param  info      message information.
 * @param  millisec  timeout value or 0 in case of no time-out.
 * @retval status code that indicates the execution status of the function.
 * @note   MUST REMAIN UNCHANGED: \b osMessagePut shall be consistent in every CMSIS-RTOS.
 */
osStatus osMessagePut(QueueHandle_t queue_id, uint32_t info, uint32_t millisec)
{
    TickType_t ticks;

    ticks = millisec / portTICK_PERIOD_MS;
    if (ticks == 0)
    {
        ticks = 1;
    }
    if (xQueueSend(queue_id, &info, ticks) != pdTRUE)
    {
        return osErrorOS;
    }
    return osOK;
}