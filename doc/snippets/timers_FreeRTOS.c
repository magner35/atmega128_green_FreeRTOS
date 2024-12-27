#include <stdlib.h>
#include <conio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
/*-----------------------------------------------------------*/
/* Количество интервальных таймеров */
#define NUMBER_OF_TIMERS 3
/* Целочисленные идентификаторы интервальных таймеров */
#define ID_TIMER_1 111
#define ID_TIMER_2 222
#define ID_TIMER_3 333
/*-----------------------------------------------------------*/
/* Дескриптор периодического таймера */
xTimerHandle xAutoReloadTimer;
/* Массив дескрипторов интервальных таймеров */
xTimerHandle xOneShotTimers[NUMBER_OF_TIMERS];
/* Массив идентификаторов интервальных таймеров */
const unsigned portBASE_TYPE uxOneShotTimersIDs
    [NUMBER_OF_TIMERS] = {ID_TIMER_1, ID_TIMER_2,
                          ID_TIMER_3};
/* Период работы периодического таймера = 1 секунда */
unsigned int uiAutoReloadTimerPeriod = 1000 / portTICK_RATE_MS;
/*-----------------------------------------------------------*/
/* Функция периодического таймера.
 * Является функцией обратного вызова.
 * В программе не должно быть ее явных вызовов.
 * В функцию автоматически передается дескриптор таймера в виде аргумента xTimer. */
void vAutoReloadTimerFunction(xTimerHandle xTimer)
{
    /* Сигнализировать о выполнении.
     * Вывести сообщение о текущем времени, прошедшем с момента запуска планировщика. */
    printf("AutoReload timer.Time = % d sec\n\r", xTaskGetTickCount() / configTICK_RATE_HZ);
    /* Увеличить период работы периодического таймера на 1 секунду */
    uiAutoReloadTimerPeriod += 1000 / portTICK_RATE_MS;
    /* Установить новый период работы периодического таймера.
     * Время тайм-аута (3-й аргумент) обязательно должно быть 0!
     * Так как внутри функции таймера нельзя вызывать блокирующие API-функции. */
    xTimerChangePeriod(xTimer, uiAutoReloadTimerPeriod, 0);
}
/*-----------------------------------------------------------*/
/* Функция интервальных таймеров.
 * Нескольким экземплярам интервальных таймеров соответствует одна-единственная функция.
 * Эта функция автоматически вызывается при истечении времени любого из связанных с ней таймеров.
 * Для того чтобы выяснить, время какого таймера истекло, используется идентификатор таймера. */
void vOneShotTimersFunction(xTimerHandle xTimer)
{
    /* Указатель на идентификатор таймера */
    unsigned portBASE_TYPE *pxTimerID;
    /* Получить идентификатор таймера, который вызывал эту функцию таймера */
    pxTimerID = pvTimerGetTimerID(xTimer);
    /* Различные действия в зависимости от того, какой таймер вызывал функцию */
    switch (*pxTimerID)
    {
    /* Сработал интервальный таймер 1 */
    case ID_TIMER_1:
        /* Индикация работы + текущее время */
        printf("\t\t\t\tOneShot timer ID = % d.Time = % d sec\n\r", *pxTimerID, xTaskGetTickCount() / configTICK_RATE_HZ);
        /* Запустить интервальный таймер 2 */
        xTimerStart(xOneShotTimers[1], 0);
        break;
    /* Сработал интервальный таймер 2 */
    case ID_TIMER_2:
        /* Индикация работы + текущее время */
        printf("\t\t\t\tOneShot timer ID = % d.Time = % d sec\n\r", *pxTimerID, xTaskGetTickCount() / configTICK_RATE_HZ);
        /* Запустить интервальный таймер 3 */
        xTimerStart(xOneShotTimers[2], 0);
        break;
    case ID_TIMER_3:
        /* Индикация работы + текущее время */
        printf("\t\t\t\tOneShot timer ID = % d.Time = % d sec\n\r", *pxTimerID,
               xTaskGetTickCount() / configTICK_RATE_HZ);
        puts("\n\r\t\t\t\tAbout to delete AutoReload timer !");
        fflush();
        /* Удалить периодический таймер.
         * После этого активных таймеров в программе не останется. */
        xTimerDelete(xAutoReloadTimer, 0);
        break;
    }
}
/*-----------------------------------------------------------*/
/* Точка входа в программу. */
short main(void)
{
    unsigned portBASE_TYPE i;
    /* Создать периодический таймер.
     * Период работы таймера = 1 секунда.
     * Идентификатор таймера не используется (0). */
    xAutoReloadTimer = xTimerCreate("AutoReloadTimer", uiAutoReloadTimerPeriod, pdTRUE, 0,
                                    vAutoReloadTimerFunction);
    /* Выполнить сброс периодического таймера ДО запуска планировщика.
     * Таким образом, он начнет отсчет времени одновременно с запуском планировщика. */
    xTimerReset(xAutoReloadTimer, 0);
    /* Создать 3 экземпляра интервальных таймеров.
     * Период работы таймеров = 12 секунд.
     * Каждому из них передать свой идентификатор.
     * Функция для них всех одна — vOneShotTimersFunction(). */
    for (i = 0; i < NUMBER_OF_TIMERS; i++)
    {
        xOneShotTimers[i] = xTimerCreate("OneShotTimer_n", 12000 / portTICK_RATE_MS, pdFALSE,
                                         (void *)&uxOneShotTimersIDs[i], vOneShotTimersFunction);
    }
    /* Выполнить сброс только первого интервального таймера.
     * Именно он начнет отсчитывать время сразу после запуска планировщика.
     * Остальные 2 таймера после запуска планировщика останутся в пассивном состоянии. */
    xTimerReset(xOneShotTimers[0], 0);
    /* Индицировать текущее время.
     * Оно будет равно 0, так как планировщик еще не запущен. */
    printf("Timers start !Time = % d sec\n\r\n\r", xTaskGetTickCount() / configTICK_RATE_HZ);
    /* Запуск планировщика.
     * Автоматически будет создана задача обслуживания таймеров.
     * Таймеры, которые были переведены в активное состояние (например, вызовом xTimerReset())
     * ДО этого момента, начнут отсчет времени. */
    vTaskStartScheduler();
    return 1;
}
/*-----------------------------------------------------------*/