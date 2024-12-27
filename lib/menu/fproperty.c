/** \file fproperty.c
 * \brief Implementation of the property system
 *
 * \author \b ARV
 * \date	29 апр. 2020 г.
 * \copyright 2020 © ARV. All rights reserved.
 *
 * Compilation requires:\n
 * 	-# \b avr-gcc \b 4.9.x or newer version
 *
 */

#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ctype.h>

#include <util/atomic.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <time.h>

#include "lcd.h"

#include <avr/delay.h>

#include "fproperty.h"

static char tmpstr[DISPLAY_WIDTH + 1];

#if defined(ENABLE_I16_PROPERTIES) || defined(ENABLE_U16_PROPERTIES)
typedef int32_t big_int;
#define ITOA(x, y, z) ltoa(x, y, z)
#else
typedef int16_t big_int;
#define ITOA(x, y, z) itoa(x, y, z)
#endif

static char *numText(big_int num)
{
	// ITOA(num, tmpstr, 10);
	sprintf(tmpstr, "%05ld", num);
	return tmpstr;
}

/*The variable width or precision field (an asterisk * symbol) is not realized
 and will to abort the output. */
static char *fnumText(float num, uint8_t intpart, uint8_t fracpart)
{
	char *format = "%+04.0f";
	format[5] = fracpart + '0';
	format[3] = intpart + '0';
	sprintf(tmpstr, format, num);
	return tmpstr;
}

static char *u32NumText(uint32_t num, uint8_t intpart)
{
	char *format = "%05lu";
	format[2] = intpart + '0';
	sprintf(tmpstr, format, num);
	return tmpstr;
}

static char *timeText(time_t num)
{
	struct tm *time;
	time = localtime(&num);
	strftime(tmpstr, DISPLAY_WIDTH, "%H:%M", time);
	return tmpstr;
}

static char *dateText(time_t num)
{
	struct tm *time;
	time = localtime(&num);
	strftime(tmpstr, DISPLAY_WIDTH, "%d/%m/%y", time);
	return tmpstr;
}

static char *hexText(big_int num, uint8_t dig)
{
	ITOA(num, tmpstr, 16);
	return tmpstr;
}

static void xStrCpy(char *dst, const __memx char *src)
{
	while (*src)
		*dst++ = *src++;
	*dst = 0;
}

char *propertyAsText(_mem property_t *prop)
{
	tmpstr[0] = 0;
	switch (prop->type)
	{
	case PR_U8:
		return numText(*(uint8_t *)prop->var);
	case PR_FLOAT:
		return fnumText(*(float *)prop->var, *((_mem prop_float_t *)prop)->intpart, *((_mem prop_float_t *)prop)->fracpart);
	case PR_U32:
		return u32NumText(*(uint32_t *)prop->var, ((_mem prop_u32_t *)prop)->intpart);
	case PR_TIME:
		return timeText(*(time_t *)prop->var);
	case PR_DATE:
		return dateText(*(time_t *)prop->var);
	case PR_U16:
		return numText(*(uint16_t *)prop->var);
	case PR_H16:
		return hexText(*(uint16_t *)prop->var, 4);
	case PR_I8:
		return numText(*(int8_t *)prop->var);
	case PR_I16:
		return numText(*(int16_t *)prop->var);
	case PR_H8:
		return hexText(*(uint8_t *)prop->var, 2);
	case PR_BOOL:
		xStrCpy(tmpstr, ((_mem prop_bool_t *)prop)->values[*(bool *)prop->var]);
		break;
	case PR_ENUM:
		xStrCpy(tmpstr, ((_mem prop_enum_t *)prop)->values[*(uint8_t *)prop->var]);
	default:
		break;
	}
	return tmpstr;
}

static big_int normalize(big_int d, big_int min, big_int max)
{
	if (d > max)
		d = max;
	else if (d < min)
		d = min;
	return d;
}

static float fnormalize(float d, float min, float max)
{
	if (d > max)
		d = max;
	else if (d < min)
		d = min;
	return d;
}

static big_int defaultVal(big_int d, big_int min, big_int max)
{
	if ((d < min) || (d > max))
	{
#if DEFAULT_PROP_VAL == DEF_MIN
		return min;
#elif DEFAULT_PROP_VAL == DEF_MAX
		return max;
#else
		return (min + max) / 2;
#endif
	}
	return d;
}

#define ATOM ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

void fpropertyEdit(_mem property_t *prop, uint8_t delta, uint8_t pos)
{
#define BUF2LEN 2
#define HOURMAX 23
#define MINUTEMAX 59
#define DAYMAX 31
#define DAYMIN 1
#define MONMAX 12
#define MONMIN 1
#define YEARMAX 99

	static char buf2[BUF2LEN + 1];
	int8_t i;
	uint8_t len;
	uint8_t tpart;
	float ftmp;
	uint32_t u32tmp;
	struct tm *edittime;
	char *buf = tmpstr;

	switch (prop->type)
	{
	case PR_FLOAT:
		buf = fnumText(*(float *)prop->var, *(uint8_t *)((_mem prop_float_t *)prop)->intpart, *(uint8_t *)((_mem prop_float_t *)prop)->fracpart);
		break;

	case PR_U32:
		buf = u32NumText(*(uint32_t *)prop->var, ((_mem prop_u32_t *)prop)->intpart);
		break;

	case PR_TIME:
		buf = timeText(*(time_t *)prop->var);
		break;

	case PR_DATE:
		buf = dateText(*(time_t *)prop->var);
		break;

	default:
		break;
	}
	len = strlen(buf);
	i = buf[len - 1 - pos];
	if (i >= '0' && i <= '9')
	{
		i = i + delta;
		// normalize range 0...9
		if (i > '9')
			i = '9';
		if (i < '0')
			i = '0';
	}
	else if (i == '+')
		i = '-';
	else if (i == '-')
		i = '+';
	buf[len - 1 - pos] = i;

	switch (prop->type)
	{
	case PR_FLOAT:
		ftmp = atof(buf);
		ftmp = fnormalize(ftmp, ((_mem prop_float_t *)prop)->min, ((_mem prop_float_t *)prop)->max);
		ATOM
		{
			*(float *)prop->var = ftmp;
		}
		break;

	case PR_U32:
		u32tmp = atol(buf);
		u32tmp = normalize(u32tmp, ((_mem prop_u32_t *)prop)->min, ((_mem prop_u32_t *)prop)->max);
		ATOM
		{
			*(uint32_t *)prop->var = u32tmp;
		}
		break;

	case PR_TIME:
		edittime = localtime((time_t *)prop->var);
		tpart = atol(strncpy(buf2, buf, 2));
		if (tpart > HOURMAX)
			tpart = HOURMAX;
		edittime->tm_hour = tpart;
		tpart = atol(strncpy(buf2, strchr(buf, ':') + 1, 2));
		if (tpart > MINUTEMAX)
			tpart = MINUTEMAX;
		edittime->tm_min = tpart;
		edittime->tm_sec = 0; // set seconds to zero
		ATOM
		{
			*(time_t *)prop->var = mktime(edittime);
		}
		break;

	case PR_DATE:
		edittime = localtime((time_t *)prop->var);
		// buf2[2] = '\0';
		tpart = atoi(strncpy(buf2, buf, BUF2LEN));
		if (tpart > DAYMAX)
			tpart = DAYMAX;
		if (tpart < DAYMIN)
			tpart = DAYMIN;
		edittime->tm_mday = tpart;
		// buf2[2] = '\0';
		tpart = atoi(strncpy(buf2, strchr(buf, '/') + 1, BUF2LEN));
		if (tpart > MONMAX)
			tpart = MONMAX;
		if (tpart < MONMIN)
			tpart = MONMIN;
		edittime->tm_mon = tpart - 1;
		// buf2[2] = '\0';
		tpart = atoi(strncpy(buf2, strrchr(buf, '/') + 1, BUF2LEN));
		if (tpart > YEARMAX)
			tpart = YEARMAX;
		edittime->tm_year = 100 + tpart;
		ATOM
		{
			*(time_t *)prop->var = mktime(edittime);
		}
		break;

	default:
		break;
	}
}

void propertyEdit(_mem property_t *prop, int16_t delta)
{
	big_int tmp;
	switch (prop->type)
	{
	case PR_U8:
	case PR_H8:
		tmp = *(uint8_t *)prop->var + delta;
		*(uint8_t *)prop->var = normalize(tmp, ((_mem prop_u8_t *)prop)->min, ((_mem prop_u8_t *)prop)->max);
		break;
	case PR_U16:
	case PR_H16:
		tmp = *(uint16_t *)prop->var + (big_int)delta;
		tmp = normalize(tmp, ((_mem prop_u16_t *)prop)->min, ((_mem prop_u16_t *)prop)->max);
		ATOM
		{
			*(uint16_t *)prop->var = tmp;
		}
		break;
	case PR_I8:
		tmp = *(int8_t *)prop->var + delta;
		*(int8_t *)prop->var = normalize(tmp, ((_mem prop_i8_t *)prop)->min, ((_mem prop_i8_t *)prop)->max);
		break;
	case PR_I16:
		tmp = *(int16_t *)prop->var + (big_int)delta;
		tmp = normalize(tmp, ((_mem prop_i16_t *)prop)->min, ((_mem prop_i16_t *)prop)->max);
		ATOM
		{
			*(int16_t *)prop->var = tmp;
		}
		break;
	case PR_BOOL:
		*(bool *)prop->var = !*(bool *)prop->var;
		break;
	case PR_ENUM:
		if (delta > 1)
			delta = 1;
		else if (delta < -1)
			delta = -1;
		tmp = *(uint8_t *)prop->var + delta;
		if (tmp >= ((_mem prop_enum_t *)prop)->cnt)
			tmp = 0;
		if (tmp < 0)
			tmp = ((_mem prop_enum_t *)prop)->cnt - 1;
		*(uint8_t *)prop->var = tmp;
		break;
	case PR_FLOAT:
		break;
	default:
		break;
	}
}

void storeProperty(_mem property_t *prop)
{
	if (prop->store == NOSTORE)
		return;
	switch (prop->type)
	{
	case PR_I8:
	case PR_U8:
	case PR_H8:
	case PR_ENUM:
		eeprom_update_block((uint8_t *)prop->var, prop->store, sizeof(uint8_t));
		break;
	case PR_I16:
	case PR_U16:
	case PR_H16:
	case PR_FLOAT:
		eeprom_update_block((uint16_t *)prop->var, prop->store, sizeof(uint16_t));
		break;
	case PR_BOOL:
		eeprom_update_block((bool *)prop->var, prop->store, sizeof(bool));
		break;
	default:
		break;
	}
}

void loadProperty(_mem property_t *prop)
{
	if (prop->store == NOSTORE)
		return;
	switch (prop->type)
	{
	case PR_U8:
	case PR_I8:
	case PR_H8:
	case PR_ENUM:
		eeprom_read_block(prop->var, prop->store, sizeof(uint8_t));
		break;
	case PR_U16:
	case PR_H16:
	case PR_I16:
	case PR_FLOAT:
		ATOM
		{
			eeprom_read_block(prop->var, prop->store, sizeof(uint16_t));
		}
		break;
	case PR_BOOL:
		eeprom_read_block(prop->var, prop->store, sizeof(bool));
		break;
	default:
		break;
	}

#if DEFAULT_PROP_VAL < DEF_ANY
	// casting to default range
	big_int tmp;

	switch (prop->type)
	{
	case PR_U8:
	case PR_H8:
		*(uint8_t *)prop->var = defaultVal(*(uint8_t *)prop->var, ((_mem prop_u8_t *)prop)->min, ((_mem prop_u8_t *)prop)->max);
		break;
	case PR_I8:
		*(int8_t *)prop->var = defaultVal(*(int8_t *)prop->var, ((_mem prop_i8_t *)prop)->min, ((_mem prop_i8_t *)prop)->max);
		break;
	case PR_ENUM:
		*(uint8_t *)prop->var = defaultVal(*(uint8_t *)prop->var, 0, ((_mem prop_enum_t *)prop)->cnt);
		break;
	case PR_BOOL:
		*(bool *)prop->var = defaultVal(*(bool *)prop->var, false, true);
		break;
	case PR_U16:
	case PR_H16:
	case PR_FLOAT:
		tmp = defaultVal(*(uint16_t *)prop->var, ((_mem prop_u16_t *)prop)->min, ((_mem prop_u16_t *)prop)->max);
		ATOM
		{
			*(uint16_t *)prop->var = tmp;
		}
		break;
	case PR_I16:
		tmp = defaultVal(*(int16_t *)prop->var, ((_mem prop_i16_t *)prop)->min, ((_mem prop_i16_t *)prop)->max);
		ATOM
		{
			*(int16_t *)prop->var = tmp;
		}
		break;
	default:
		break;
	}
#endif
}
