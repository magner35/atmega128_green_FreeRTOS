// -----
// OneButton.c - библиотека для определения нажатий на кнопку,
// двойных нажатий и долгих нажатий. Это порт для проектов на
// языке C (WinAVR, AVR Studio).
// Код защищен лицензией стиля BSD, см.:
// http://www.mathertel.de/License.aspx
// Дополнительная информация:
// http://www.mathertel.de/Arduino
// -----
// Историю изменения библиотеки см. в файле OneButton.h.
// -----

/* Порт библиотеки OneButton для проектов на языке C (WinAVR, AVR Studio)
Отличие порта от оригинального класса OneButton в том, что отсутствует конструктор,
настраивающий ножку порта и его активный уровень он (отсутствуют переменные _buttonReleased и
_buttonPressed). Предполагается, что активный уровень кнопки низкий (замыкание кнопки осуществляется
на землю, GND), и должны присутствовать внешние макроопределения для имен регистров и ножки порта
(DDR_Enc, PORT_Enc, Btn_Enc). Также должен быть миллисекундный счетчик относительного реального
времени timestamp, обновляемый с помощью обработчика прерывания таймера или другим способом.
Машина конечных состояний обрабатывается процедурой OBtick, которая должна вызываться с
интервалами примерно 10 миллисекунд. */

#include <stddef.h>
#include <avr/io.h>

#include "onebutton.h"

extern uint16_t timestamp;

int _clickTicks;              // Количество тиков, которое должно пройти, чтобы
                              // было засчитано короткое нажатие на кнопку.
int _pressTicks;              // Количество тиков, которое должно пройти, чтобы
                              // было засчитано длинное нажатие на кнопку.
const int _debounceTicks = 1; // количество тиков для подавления дребезга.bool _isLongPressed;
// Переменные ниже хранят информацию машины состояний, сохраняющуюся
// между отдельными вызовами tick(). Они инициализируются один раз
// при старте программы, и обновляются каждый раз при вызове функции
// tick().
uint8_t _state[8];        // максимальное количество кнопок
unsigned long _startTime; // Эта переменная будет установлена в state 1.

volatile bool _isLongPressed;

void OBInit(void)
{
    _clickTicks = 5;  // Количество тиков, которое должно пройти, чтобы
                       // было засчитано короткое нажатие на кнопку.
    _pressTicks = 150; // Количество тиков, которое должно пройти, чтобы
                       // было засчитано длинное нажатие на кнопку.

    for (uint8_t i = 0; i < 7; i++) // Начальное состояние state: в нем ожидается первое нажатие на кнопку.
        _state[i] = 0;

    _isLongPressed = false; // Флаг, отслеживающий долгое нажатие.
    _doubleClickFunc = NULL;
    _pressFunc = NULL;
    _longPressStartFunc = NULL;
    _longPressStopFunc = NULL;
    _duringLongPressFunc = NULL;
}

// Установка количества тиков (или миллисекунд, если вызовы tick происходят
// раз в миллисекунду), которое должно пройти, чтобы было детектировано
// короткое нажатие на кнопку.
void OBSetClickTicks(int ticks)
{
    _clickTicks = ticks;
}

// Установка количества тиков (или миллисекунд, если вызовы tick происходят
// раз в миллисекунду), которое должно пройти, чтобы было детектировано
// долгое по времени нажатие на кнопку.
void OBSetPressTicks(int ticks)
{
    _pressTicks = ticks;
}

// Сохранение указателя на функцию для обработки события клика.
void OBAttachClick(callbackFunction newFunction)
{
    _clickFunc = newFunction;
}

// Сохранение указателя на функцию для обработки события двойного клика.
void OBAttachDoubleClick(callbackFunction newFunction)
{
    _doubleClickFunc = newFunction;
}

// Сохранение указателя на функцию для обработки события начала
// длинного нажатия.
void OBAttachLongPressStart(callbackFunction newFunction)
{
    _longPressStartFunc = newFunction;
}

// Сохранение указателя на функцию для обработки события завершения
// длинного нажатия.
void OBAttachLongPressStop(callbackFunction newFunction)
{
    _longPressStopFunc = newFunction;
}

// Сохранение указателя на функцию для обработки события
// длинного нажатия.
void OBAttachDuringLongPress(callbackFunction newFunction)
{
    _duringLongPressFunc = newFunction;
}

// Функция для получения состояния длинного нажатия:
bool OBIsLongPressed()
{
    return _isLongPressed;
}

void OBtick(bool buttonLevel, uint8_t button)
{
    button = button & 0x03;   // берем индекс по маске
    uint32_t now = timestamp; // текущее (относительное) время в миллисекундах.
    // Реализация машины состояний
    if (_state[button] == 0)
    { // Ожидание нажатия на кнопку.
        if (buttonLevel == false)
        {
            _state[button] = 1; // переход в состояние state 1
            _startTime = now;   // запоминание точки отсчета времени
        }
    }
    else if (_state[button] == 1)
    { // Ожидание отпускания кнопки.
        if ((buttonLevel == true) && (now < _startTime + _debounceTicks))
        {
            // Кнопка была отпущена достаточно быстро, чтобы можно было
            // считать это дребезгом контактов. Поэтому происходит
            // переход в начальное состояние, без каких-либо вызовов функций.
            _state[button] = 0;
        }
        else if (buttonLevel == true)
        {
            _state[button] = 2; // переход в состояние state 2
        }
        else if ((buttonLevel == false) && (now > _startTime + _pressTicks))
        {
            _isLongPressed = true; // зарегистрировать долгое нажатие
            if (_pressFunc)
                _pressFunc(button);
            if (_longPressStartFunc)
                _longPressStartFunc(button);
            if (_duringLongPressFunc)
                _duringLongPressFunc(button);
            _state[button] = 6; // переход в состояние state 6
        }
        else
        {
            // Ожидание, пока остаемся в этом состоянии.
        }
    }
    else if (_state[button] == 2)
    {
        // Ожидание вторичного нажатия на кнопку или истечения таймаута.
        if (now > _startTime + _clickTicks)
        {
            // Это было простое короткое нажатие
            if (_clickFunc)
                _clickFunc(button);
            _state[button] = 0; // возврат в исходное состояние.
        }
        else if (buttonLevel == false)
        {
            _state[button] = 3; // переход в состояние state 3
        }
    }
    else if (_state[button] == 3)
    { // Ожидание завершающего опускания кнопки.
        if (buttonLevel == true)
        {
            // this was a 2 click sequence.
            if (_doubleClickFunc)
                _doubleClickFunc(button);
            _state[button] = 0; // возврат в исходное состояние.
        }
    }
    else if (_state[button] == 6)
    { // Ожидание опускания кнопки после долгого нажатия.
        if (buttonLevel == true)
        {
            _isLongPressed = false; // Отслеживание состояния долгого нажатия.
            if (_longPressStopFunc)
                _longPressStopFunc(button);
            _state[button] = 0; // возврат в исходное состояние.
        }
        else
        {
            // Кнопка была нажата длительное время.
            _isLongPressed = true; // Отслеживание состояния долгого нажатия.
            if (_duringLongPressFunc)
                _duringLongPressFunc(button);
        }
    }
}