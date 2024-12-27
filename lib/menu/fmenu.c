/** \file fmenu.c
 * \brief Реализация системы меню
 *
 * \author \b ARV
 * \date	29 апр. 2020 г.
 * \copyright 2020 © ARV. All rights reserved.
 *
 * Для компиляции требуется:\n
 * 	-# \b avr-gcc \b 4.9.x или более новая версия
 *
 */

#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "fmenu.h"
#include "fproperty.h"

enum
{
	MMODE_MAIN,
	MMODE_SWITCH,
	MMODE_EDIT
};

_mem menu_item_t NONE = {0};
_mem menu_item_t *menu_first;
_mem menu_item_t *menu_current = &NONE;

// power function
uint32_t myPow(uint32_t x, uint8_t n)
{
	long int a = x, p = 1;
	while (n > 0)
	{
		if ((n & 1) != 0)
			p *= a;
		a *= a;
		n >>= 1;
	}
	return p;
}

static void strCpy(char *dest, const __memx char *src, uint8_t len)
{
	while (len-- && *src)
		*dest++ = *src++;
}

void getItemText(char *buf, _mem menu_item_t *item)
{
	char *tmp;
	memset(buf, ' ', MENU_TEXT_LEN);
	buf[MENU_TEXT_LEN] = 0;

	if (item != &NONE)
	{
		// имя пункта слева
		strCpy(buf + SPACE_LEFT, item->name, MENU_TEXT_LEN);
		switch (item->type)
		{
		case MITEM_PROP:
			// представление свойства - справа
			tmp = propertyAsText(item->prop);
			strCpy(buf + MENU_TEXT_LEN - SPACE_RIGHT - strlen(tmp), tmp, PROP_WIDTH);
			break;

		case MITEM_SUBM:
			// субменю отмечается спецсимволом
			buf[MENU_TEXT_LEN - 1 - SPACE_RIGHT] = SUBMENU_SYMBOL;
			break;
		}
	}
}

/// \note функция рекурсивно обходит дерево меню, поэтому не стоит увлекаться
/// глубиной вложенности субменю, чтобы не переполнить стек
void storeMenuProperties(_mem menu_item_t *item)
{
	while (item->next != &NONE)
	{
		// ищем пункты со свойствами
		if (item->type == MITEM_PROP)
		{
			// и сохраняем найденные свойства
			storeProperty(item->prop);
		}
		else if (item->type == MITEM_SUBM)
		{
			// погружаемся на уровень субменю и сохраняем его свойства
			storeMenuProperties(item->sub);
		}
		item = item->next;
	}
}

static bool notifyProp(_mem menu_item_t *pm)
{
	if (pm->func != NULL)
		return pm->func(pm->type == MITEM_CMD ? NULL : ((_mem property_t *)pm->prop)->var);
	else
		return true;
}

/// \note функция рекурсивно обходит дерево меню, поэтому не стоит увлекаться
/// глубиной вложенности субменю, чтобы не переполнить стек
void loadMenuProperties(_mem menu_item_t *item)
{
	while (item->next != &NONE)
	{
		// ищем пункты со свойствами
		if (item->type == MITEM_PROP)
		{
			// загружаем найденные
			loadProperty(item->prop);
			notifyProp(item);
		}
		else if (item->type == MITEM_SUBM)
		{
			// для субменю повторяем то же самое
			loadMenuProperties(item->sub);
		}
		item = item->next;
	}
}

///\note функция выводит меню
void initMenu(_mem menu_item_t *menu, bool load_props)
{
	if (load_props)
		loadMenuProperties(menu);
	menu_current = menu;
	menu_first = &NONE;
	renderMenu();
}

bool doMenu(menu_event_t event)
{
	static uint8_t menu_mode;
	static uint8_t edit_position;
	static uint8_t point_position;
	static uint8_t edit_field_length;
	static uint32_t edit_old_value;

	if (menu_mode == MMODE_MAIN)
	{
		if ((menu_current == NULL) || (menu_current == &NONE))
			return false;
		switch (event)
		{
		case MEV_ENTER:
			switch (menu_current->type)
			{
			case MITEM_CMD:
				if (notifyProp(menu_current))
				{
					break;
				}
				return false;

			case MITEM_PROP:
				break;

			case MITEM_SUBM:
				menu_current = menu_current->sub;
				menu_first = &NONE; // menu_current;
			}
			renderMenu();
			break;

		case MEV_ESCAPE:
			if ((*menu_current->parent).parent != &NONE)
			{
				menu_first = &NONE;
				menu_current = menu_current->parent; // menu_first;
				renderMenu();
			}
			else
			{
				renderMenu();
				return false;
			}
			break;

		case MEV_NEXT:
			if (menu_current->next != &NONE)
			{
				menu_current = menu_current->next;
				renderMenu();
			}
			break;

		case MEV_PREV:
			if (menu_current->prev != &NONE)
			{
				menu_current = menu_current->prev;
				renderMenu();
			}
			break;

		case MEV_EDIT:
			if (menu_current->type == MITEM_PROP)
			{
				menu_mode = MMODE_EDIT;
				edit_position = 0;
				edit_field_length = strlen(propertyAsText(menu_current->prop));
				edit_old_value = *(uint32_t *)((_mem property_t *)menu_current->prop)->var;
				switch (((_mem property_t *)menu_current->prop)->type)
				{
				case PR_FLOAT:
					point_position = *(uint8_t *)((_mem prop_float_t *)menu_current->prop)->fracpart;
					break;
				case PR_U32:
					point_position = 0;
					break;
				case PR_TIME:
					point_position = 2; // HH:MM colon position
					break;
				case PR_DATE:
					point_position = 2; // DD/MM/YY slash position
					break;
				default:
					menu_mode = MMODE_SWITCH;
					break;
				}
			}
			break;

		case MEV_NONE:
		default:
			break;
		}
	}

	else if (menu_mode == MMODE_SWITCH)
	{
		switch (event)
		{
		case MEV_ENTER:
			break;

		case MEV_EDIT:
			menu_mode = MMODE_MAIN;
			notifyProp(menu_current);
			renderMenu();
			return true;
			break;

		case MEV_ESCAPE:
			menu_mode = MMODE_MAIN;
			*(uint32_t *)((_mem property_t *)menu_current->prop)->var = edit_old_value;
			renderMenu();
			return true;
			break;

		case MEV_NEXT:
			propertyEdit(menu_current->prop, 1);
			break;

		case MEV_PREV:
			propertyEdit(menu_current->prop, -1);
			break;

		case MEV_NONE:
		default:
			break;
		}
		renderSwitchField();
		return true;
	}

	else if (menu_mode == MMODE_EDIT)
	{
		switch (event)
		{
		case MEV_ENTER:
			edit_position++;
			// decimal point skip
			if (edit_position == point_position)
				edit_position++;

			if ((((_mem property_t *)menu_current->prop)->type) == PR_DATE)
			{
				if (edit_position == 5)
					edit_position++;
			}
			if (edit_position == edit_field_length)
			{
				edit_position = 0;
			}
			break;

		case MEV_NEXT:
			fpropertyEdit(menu_current->prop, -1, edit_position);
			break;

		case MEV_PREV:
			fpropertyEdit(menu_current->prop, 1, edit_position);
			break;

		case MEV_EDIT:
			menu_mode = MMODE_MAIN;
			notifyProp(menu_current);
			renderMenu();
			return true;
			break;

		case MEV_ESCAPE:
			menu_mode = MMODE_MAIN;
			*(uint32_t *)((_mem property_t *)menu_current->prop)->var = edit_old_value;
			renderMenu();
			return true;
			break;

		case MEV_NONE:
		default:
			break;
		}
		renderEditField(edit_position);
		return true;
	}
	return true;
}
