#ifndef _ONEBUTTON_H_
#define _ONEBUTTON_H_
#include <inttypes.h>

#include <stdbool.h>

void OBInit(void);

typedef enum
{
    OB_NONE,    
    OB_LONGPRESSSTART,
    OB_CLICK,
    OB_DOUBLECLICK,
    OB_LONGPRESSSTOP,
    OB_DURINGLONGPRESS
} button_state;

// ----- Функции машины состояний -----
// Функция tick должна вызываться периодически, с интервалом времени
// порядка 10 мс - чтобы библиотека могла обработать события кнопки.
uint8_t OBtick(bool, uint8_t);

#endif // _ONEBUTTON_H_
