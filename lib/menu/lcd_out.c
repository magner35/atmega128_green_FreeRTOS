/** \file lcd_out.c
 * \brief Модуль вывода меню на символьный ЖКИ
 *
 * \details
 * Поддержка вывода на символьный ЖКИ реализуется сторонней библиотекой \ref lcd.c
 *
 * \author \b ARV
 * \date	5 мая 2020 г.
 * \copyright 2020 © ARV. All rights reserved.
 *
 */

#include <avr/io.h>
#include <string.h>

#include "lcd.h"
#include "fmenu.h"
#include "fproperty.h"

static _mem menu_item_t *lines[MENU_HEIGHT];
static char text[DISPLAY_WIDTH + 1];

static bool visibled(_mem menu_item_t *from, _mem menu_item_t *item)
{
	for (uint8_t i = 0; i < MENU_HEIGHT; i++)
	{
		if (item == from)
			return true;
		if (from != &NONE)
			from = from->next;
	}
	return false;
}

static void fill_lines(_mem menu_item_t *from)
{
	for (uint8_t i = 0; i < MENU_HEIGHT; i++)
	{
		lines[i] = from;
		if (from != &NONE)
			from = from->next;
	}
}

void renderMenu()
{
	_mem char *title;
	lcd_gotoxy(0, 0);
	lcd_puts_p(PSTR("===================="));
	title = menu_current->parent->name;
	lcd_gotoxy((DISPLAY_WIDTH / 2) - strlen_P(title) / 2, 0);
	lcd_puts_p(title);

	// if a menu/submenu has changed
	if (menu_first == &NONE)
	{
		menu_first = menu_current;
		fill_lines(menu_current);
	}
	// search for menu items that need to be displayed so that the current item is visible
	if (!visibled(lines[0], menu_current))
	{
		if (visibled(lines[0]->prev, menu_current))
			fill_lines(lines[0]->prev);
		else
			fill_lines(lines[0]->next);
	}
	// display visible menu items
	for (uint8_t y = 0; y < MENU_HEIGHT; y++)
	{
		getItemText(text, lines[y]);
		if (lines[y] == menu_current)
		{
			text[0] = '>';
		}
		lcd_gotoxy(0, y + MENU_SHIFT);
		lcd_puts(text);
	}
}

void renderSwitchField()
{
	static uint16_t blink_period;
	uint8_t k;
	for (uint8_t y = 0; y < MENU_HEIGHT; y++)
	{
		if (lines[y] == menu_current)
		{
			getItemText(text, lines[y]);
			text[0] = '>';
			if (blink_period == 0)
			{
				lcd_gotoxy(0, y + MENU_SHIFT);
				lcd_puts(text);
			}
			if (blink_period == 8)
			{
				k = DISPLAY_WIDTH - strlen(propertyAsText(menu_current->prop)); // fill with spaces - blinking implementation
				for (uint8_t i = k; i < DISPLAY_WIDTH; i++)
				{
					text[i] = ' ';
				}
				lcd_gotoxy(0, y + MENU_SHIFT);
				lcd_puts(text);
			}
		}
	}
	if (++blink_period == 10)
	{
		blink_period = 0;
	}
}

void renderEditField(uint8_t blinkpos)
{
	static uint16_t blink_period;
	for (uint8_t y = 0; y < MENU_HEIGHT; y++)
	{
		if (lines[y] == menu_current)
		{
			getItemText(text, lines[y]);
			text[0] = '>';
			if (blink_period == 0)
			{
				lcd_gotoxy(0, y + MENU_SHIFT);
				lcd_puts(text);
			}
			if (blink_period == 8)
			{
				text[DISPLAY_WIDTH - 1 - blinkpos] = ' '; // blank pos
				lcd_gotoxy(0, y + MENU_SHIFT);
				lcd_puts(text);
			}
		}
	}
	if (++blink_period == 10)
	{
		blink_period = 0;
	}
}
