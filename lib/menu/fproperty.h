/** \file fproperty.h
 * \brief Определения системы свойств
 * \defgroup PROPERTY Концепция свойств
 * \brief Описание концепции свойств
 * \details
 * Концепция "свойств" расширяет понятие переменной, хранящей какое-либо "настраиваемое" значение
 * для разрабатываемого устройства. Работа со свойствами, а не с переменными, освобождает разработчика от
 * рутинных действий по интерактивному изменению значений при помощи системы меню, сохранению параметров
 * в \b EEPROM и/или считыванию их оттуда.
 *
 * Реализованы следующие типы свойств:
 * \li Числа 8-битные и 16-битные с представлением в виде дестичных чисел или HEX (см. #PROP_U8, #PROP_I8, #PROP_U16, #PROP_I16,
 * #PROP_H8, #PROP_H16)
 * \li Логическое (булево) значение (см. #PROP_BOOL)
 * \li Выбор из списка вариантов (см. #PROP_ENUM)
 *
 * Свойство характеризуется несколькими параметрами, отличающимися в зависимости от типа свойства.
 * Например, числовое свойство характеризуется границами допустимых значений (минимум и максимум),
 * а логическое (булево) характеризуется текстом, который будет показан при интерактивном редактировании
 * значения.
 *
 * Использование свойств очень несложно:
 * \li Определение переменных, хранящих значения, резервирование областей \b \b EEPROM для сохранения их значений
 * \li Определение свойств при помощи вспомогательных макросов #PROP_U8, #PROP_I8, #PROP_U16, #PROP_I16,
 * #PROP_H8, #PROP_H16. При необходимости дополнительно определяются
 * массивы с текстовыми константами для представления значений свойств.
 * \li Интеграцию свойств в систему интерактивного меню (см. \ref MENU).
 *
 * Для экономии \b ОЗУ все свойства и связанные с ними строковые константы размещаются в памяти программ.
 * \code
 * #include "fproperty.h"
 * #include <avr/eeprom.h>
 *
 * uint8_t bright; // заданная яркость
 * EEMEM uint8_t e_bright; // резервирование памяти \b EEPROM для хранения заданной яркости
 * bool power; // режим питания нагрузки
 *
 * _mem pchar_f bool_txt[] = {
 *	(str_f){"OFF"}, // строка для значения false
 *	(str_f){"ON"}	// строка для значения true
 * };
 *
 * _mem property_t* _mem prop_array[] = {
 *	(_mem property_t *)&PROP_U8(bright, &e_bright, 0, 12),
 *	(_mem property_t *)&PROP_BOOL(power, NOSTORE, bool_txt)
 * };
 *
 * \endcode
 * В показанном примере создается массив из 2-x свойств: целочисленного беззнакового значения в пределах от 0 до 12 и
 * булева, отображаемого текстом \b OFF/ON соответственно для \b false/true . Значение \b #NOSTORE показвает,
 * что сохранение значений свойств в \b \b EEPROM не реализуется.
 *
 * Описанные в виде массива свойства могут передаваться в систему меню именно как элементы массива, т.е.
 * примерно так:
 * \code MITEM_PROP(mm_0, "TEST",	NONE, NONE,	NONE, prop_array[0]); \endcode
 *
 * Можно передавать свойства в систему меню напрямую, без оформления их в виде отдельных
 * структурных единиц программы, например, так:
 * \code
 * int16_t foo;
 *
 * MITEM_PROP(mm_0, "TEST",	NONE, NONE,	NONE, PROP_I16(foo, NOSTORE, -20, 20)); \endcode
 *
 * Переменные, связанные со свойствами, используются в программе, как обычно. Поскольку их назначение -
 * хранить "конфигурационные" данные программы, не рекомендуется изменять их значение непосредственно. Для
 * изменения этих переменных следует использовать систему меню, реализующую процесс интерактивного редактирования
 * этих значений.
 * \note Если вам не требуется система меню, выигрыша от системы свойств получить не получится никакого.
 *
 * \author \b ARV
 * \date	29 апр. 2020 г.
 * \copyright 2020 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# \b avr-gcc \b 4.9.x или более новая версия
 *
 */

#ifndef _FPROPERTY_H_
#define _FPROPERTY_H_

#include <stdbool.h>

#include "fmenu_config.h"

/// тип свойства
typedef enum
{
	PR_FLOAT, //!< float point
	PR_U32,	  //!< unsigned 32 bit
	PR_TIME,  //!< time HH:MM
	PR_DATE,  //!< date DD/MM/YY
	PR_U8,	  //!< целое 8 бит без знака
	PR_I8,	  //!< целое 8 бит со знаком
	PR_H8,	  //!< целое 8 бит в виде HEX
#if !__DOXYGEN__
	PR_U16, //!< целое 16 бит без знака
	PR_H16, //!< целое 16 бит в виде HEX
#endif
#if !__DOXYGEN__
	PR_I16, //!< целое 16 бит со знаком
#endif
#if !__DOXYGEN__
	PR_BOOL, //!< булево
#endif
#if !__DOXYGEN__
	PR_ENUM //!< вариант из перечисления
#endif
} prop_type_t;

/**
 * \anchor Общие_поля
 * \par Общие поля всех свойств:
 * \b type тип свойства \n
 * \b var указатель на переменную со значением \n
 * \b store адрес сохранения в \b \b EEPROM
 */
#define PROPERTY_FIELDS \
	prop_type_t type;   \
	void *var;          \
	void *store

/// обобщенное свойство
typedef struct
{
	PROPERTY_FIELDS; //!< см. \ref Общие_поля
} property_t;

/// Определение свойства: плавающая точка
typedef struct
{
	PROPERTY_FIELDS;
	float min;		//!< минимально допустимое значение
	float max;		//!< максимально допустимое значение
	uint8_t *intpart;	// integer part
	uint8_t *fracpart; // fractional part
} prop_float_t;

/// Определение свойства: целое 32бит без знака
typedef struct
{
	PROPERTY_FIELDS;
	uint32_t min;	  //!< минимально допустимое значение
	uint32_t max;	  //!< максимально допустимое значение
	uint8_t intpart; // integer part
} prop_u32_t;

/// Определение свойства: Время в формате ЧЧ:ММ
typedef struct
{
	PROPERTY_FIELDS;
} prop_time_t;

/// Определение свойства: Дата в формате ДД/ММ/ГГ
typedef struct
{
	PROPERTY_FIELDS;
} prop_date_t;

/// Определение свойства: целое 8 бит без знака
typedef struct
{
	PROPERTY_FIELDS;
	uint8_t min; //!< минимально допустимое значение
	uint8_t max; //!< максимально допустимое значение
} prop_u8_t;

/// Определение свойства:  целое 8 бит со знаком
typedef struct
{
	PROPERTY_FIELDS;
	int8_t min; //!< минимально допустимое значение
	int8_t max; //!< максимально допустимое значение
} prop_i8_t;

/// Определение свойства:  целое 16 бит без знака
typedef struct
{
	PROPERTY_FIELDS;
	uint16_t min; //!< минимально допустимое значение
	uint16_t max; //!< максимально допустимое значение
} prop_u16_t;

/// Определение свойства:  целое 16 бит со знаком
typedef struct
{
	PROPERTY_FIELDS;
	int16_t min; //!< минимально допустимое значение
	int16_t max; //!< максимально допустимое значение
} prop_i16_t;

/// Определение свойства:  вариант выбора
typedef struct
{
	PROPERTY_FIELDS;
	uint8_t cnt;		  //!< количество вариантов для выбора
	_mem pchar_f *values; //!< массив строк-вариантов (см. #PROP_BOOL)
} prop_enum_t;

/// Определение свойства:  булево
typedef prop_enum_t prop_bool_t;

// вспомогательные макросы для удобного формирования параметров свойств

/**  \ingroup MACRO
 * Определение свойства: число c плавающей точкой \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение \т
 * \b _intpart количество знаков до точки (здесь - общее количество знаков)\т
 * \и _fracpart количество знаков после точки
 */
#define PROP_FLOAT(_var, _eadr, _min, _max, _intpart, _fracpart) \
	(_mem prop_float_t) { .type = PR_FLOAT, .var = &_var, .store = _eadr, .min = _min, .max = _max, .intpart = &_intpart, .fracpart = &_fracpart }

/** \ingroup MACRO
 *  Определение свойства: число 32 бит без знака \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение \n
 * \b _intpart количество выводимых знаков
 */
#define PROP_U32(_var, _eadr, _min, _max, _intpart) \
	(_mem prop_u32_t) { .type = PR_U32, .var = &_var, .store = _eadr, .min = _min, .max = _max, .intpart = _intpart }

/** \ingroup MACRO
 *  Определение свойства: Время в формате ЧЧ:ММ \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 */
#define PROP_TIME(_var, _eadr) \
	(_mem prop_time_t) { .type = PR_TIME, .var = &_var, .store = _eadr }

/** \ingroup MACRO
 *  Определение свойства: Дата в формате ДД/ММ/ГГ \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 */
#define PROP_DATE(_var, _eadr) \
	(_mem prop_date_t) { .type = PR_DATE, .var = &_var, .store = _eadr }

/**  \ingroup MACRO
 * Определение свойства: число 8 бит без знака \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_U8(_var, _eadr, _min, _max) \
	(_mem prop_u8_t) { .type = PR_U8, .var = &_var, .store = _eadr, .min = _min, .max = _max }

/** \ingroup MACRO
 *  Определение свойства: число 8 бит со знаком \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_I8(_var, _eadr, _min, _max) \
	(_mem prop_i8_t) { .type = PR_I8, .var = &_var, .store = _eadr, .min = _min, .max = _max }

/** \ingroup MACRO
 *  Определение свойства: число 16 бит без знака \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_U16(_var, _eadr, _min, _max) \
	(_mem prop_u16_t) { .type = PR_U16, .var = &_var, .store = _eadr, .min = _min, .max = _max }

/** \ingroup MACRO
 *  Определение свойства: число 16 бит без знака, отображаемое в HEX \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_H16(_var, _eadr, _min, _max) \
	(_mem prop_u16_t) { .type = PR_H16, .var = &_var, .store = _eadr, .min = _min, .max = _max }

/** \ingroup MACRO
 *  Определение свойства: число 16 бит со знаком \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_I16(_var, _eadr, _min, _max) \
	(_mem prop_i16_t) { .type = PR_I16, .var = &_var, .store = _eadr, .min = _min, .max = _max }

/** \ingroup MACRO
 *  Определение свойства: число 8 бит без знака, отображаемое в HEX\n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _min минимально допустимое значение \n
 * \b _max максимально допустимое значение
 */
#define PROP_H8(_var, _eadr, _min, _max) \
	(_mem prop_u8_t) { .type = PR_H8, .var = &_var, .store = _eadr, .min = _min, .max = _max }

#define fs(...) ((_mem pchar_f[]){__VA_ARGS__})

/** \ingroup MACRO
 *  Определение свойства: булево значение \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _values массив типа \b _mem \b pchar_f[] , хранящий стоки, соответствующие значениям свойства (см. пример) \n
 * \code
 * _mem pchar_f bool_txt[] = {
 *	(str_f){"OFF"}, // строка для значения false
 *	(str_f){"ON"}	// строка для значения true
 * };
 * \endcode
 * \note количество элементов этого массива должно быть всегда равно 2
 */
#define PROP_BOOL(_var, _eadr, ...) \
	(_mem prop_bool_t) { .type = PR_BOOL, .var = &_var, .store = _eadr, .cnt = 2, .values = fs(__VA_ARGS__) }

/** \ingroup MACRO
 *  Определение свойства: вариант из перечисления \n
 * \b _var имя переменной \n
 * \b _eadr адрес переменной в \b EEPROM для сохранения значения свойства \n
 * \b _cnt количество вариантов
 * \b _values массив типа \b _mem \b pchar_f[] , хранящий стоки, соответствующие значениям свойства (см. пример для #PROP_BOOL) \n
 * \note количество элементов в этом массиве должно быть равно количеству вариантов \b _cnt !
 */
#define PROP_ENUM(_var, _eadr, _cnt, ...) \
	(_mem prop_enum_t) { .type = PR_ENUM, .var = &_var, .store = _eadr, .cnt = _cnt, .values = fs(__VA_ARGS__) }

/**
 * текстовое представление свойства
 * @param prop указатель на свойство
 * @return указатель на текстовый вид свойства
 */
char *propertyAsText(_mem property_t *prop);

/**
 * редактор свойства
 * @param prop указатель на свойство
 * @param delta приращение значения свойства
 * \note для булева свойства значение \b delta игнорируется
 */
void propertyEdit(_mem property_t *prop, int16_t delta);

/**
 * редактор свойства для float
 * @param prop указатель на свойство
 * @param delta приращение значения свойства
 * @param pos редактируемая позиция
 */
void fpropertyEdit(_mem property_t *prop, uint8_t delta, uint8_t pos);

// варианты приведения значения свойства к допустимому диапазону после считывания
/// минимально допустимое
#define DEF_MIN 0
/// максимально допустимое
#define DEF_MAX 1
/// среднее между минимумом и максимумом
#define DEF_MID 2
/// не нормировать (опасно!)
#define DEF_ANY 3

/// режим нормирования считанных из \b EEPROM значений свойств:\n
/// один из #DEF_MIN, #DEF_MAX, #DEF_MID или #DEF_ANY
/// \note нормирование по умолчанию используется при считывании значений
/// свойств функцией #loadProperty в том случае, если из \b EEPROM считано
/// значение, не попадающее в допустимый диапазон (такое бывает, например, при первом
/// запуске, когда содержимое \b EEPROM еще не инициализировано)
#define DEFAULT_PROP_VAL DEF_MIN

/**
 * считывание значения свойства из \b EEPROM
 * @param prop считываемое свойство
 */
void loadProperty(_mem property_t *prop);

/**
 * запись значения свойства в \b EEPROM
 * @param prop записываемое свойство
 */
void storeProperty(_mem property_t *prop);

/// свойства, значение которых не должно сохраняться, должны получить это значение в качестве адреса \b EEPROM
#define NOSTORE ((void *)-1)

#endif /* _FPROPERTY_H_ */
