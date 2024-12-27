//==================
#include "dmenu.h"
//==================
#include <stdint.h>
//==================
static uint8_t quant_items;
static uint8_t pos_y_curs;
//==================

//================
menu_item __flash *CurrMenuItem; // Текущий пункт меню.

menu_item __flash *BeginCurrMenuLevel; // Начало массива текущего уровня меню.

menu_item __flash *temp_menu;

menu_item __flash Null_Menu = {(void *)0, (void *)0, (void *)0, (void *)0, NULL_FUNC, NULL_FUNC, NULL_FUNC, NULL_FUNC, NULL_TEXT, NULL_TEXT};
//================

//==================
bool Set_Menu_Level(menu_item __flash *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return false;
    else
    {
        CurrMenuItem = NewMenu;

        Out_Menu_Items_Init(); // Так как новый уровень, инициализация переменных.
        Out_Menu_Items();      // Вывод названия уровня меню и пунктов меню, курсора.

        GO_MENU_FUNC(MENU_INIT_FUNC);
        return true;
    }
}
//==================

//==================
bool MenuFunc(FuncPtr *Function)
{
    if ((void *)Function == (void *)NULL_FUNC)
        return false;
    else
    {
        ((FuncPtr)Function)();
        return true;
    }
}
//==================

/*
Уровни, пункты, текст - все выводится автоматом.
Так как все переходы по меню расписаны в структуре, то отпадает надобность в запоминании перемещений по меню.
*/

//==================
void Out_Menu_Items_Init(void)
{
    quant_items = 1;
    pos_y_curs = 1;

    // Получение адреса начала массива уровня меню.
    BeginCurrMenuLevel = CurrMenuItem;
    temp_menu = (menu_item __flash *)(CurrMenuItem->Prev);

    while (1)
    {
        if ((void *)temp_menu == (void *)&NULL_ENTRY)
        {
            break;
        }
        else
        {
            BeginCurrMenuLevel = temp_menu;
            temp_menu = (menu_item __flash *)(temp_menu->Prev);
        }
    }

    // Получение количества пунктов меню.
    temp_menu = (menu_item __flash *)(BeginCurrMenuLevel->Next);

    while (1)
    {
        if ((void *)temp_menu == (void *)&NULL_ENTRY)
        {
            break;
        }

        temp_menu = (menu_item __flash *)(temp_menu->Next);
        quant_items++;
    }

    // Позиция курсора.
    if (quant_items > 1)
    {
        temp_menu = BeginCurrMenuLevel;

        while (1)
        {
            if ((void *)temp_menu == (void *)&NULL_ENTRY)
                return;

            if (temp_menu == CurrMenuItem)
                return;
            else
                pos_y_curs++;

            temp_menu = (menu_item __flash *)(temp_menu->Next);
        }
    }
}

void Out_Menu_Items(void)
{
    clr_dsp_buf();

    out_name_level(); // Вывод названия уровня меню.

    make_page_menu(); // Вывод пунктов меню.

    set_pos_curs(); // Установка позиции и вывод курсора.
}
//==================

//==================
// Вывод названия уровня меню.
void out_name_level(void)
{
    temp_menu = (menu_item __flash *)(CurrMenuItem->Parent); // Считывание названия уровня меню из пункта меню в верхнем уровне.

    if ((void *)temp_menu != (void *)&NULL_ENTRY)
    {
        uint8_t i = count_chars(Get_Addr_Lang_Text()); // Подсчет кол-ва символов в строке.

        // Выравнивание текста посередине строки дисплея.

        uint8_t a = i;

        i = (20 - i); // Дисплей 20x4. Отнимаем от 20 число символов.

        i >>= 1; // Делим остаток на 2.

        if (a & (1 << 0))
            i += 2; // Если число нечетное.
        else
            i++; // Если число четное.

        Print_Buf(1, i, Get_Addr_Lang_Text());
    }
}
//==================

//==================
// Подсчет кол-ва символов в строке.
uint8_t count_chars(char __flash *data)
{
    uint8_t i = 0;

    while (data[i])
    {
        i++;
    }
    return i;
}
//==================

//==================
void make_page_menu(void)
{
    signed char tmp_pos_y_curs;
    uint8_t i; // Счетчик страниц.
    uint8_t j; // Страница меню.

    if (quant_items > 1) // Если пунктов меню больше 1, значит есть что выводить.
    {
        temp_menu = BeginCurrMenuLevel;

        if (pos_y_curs > PAGE_MENU)
        {
            tmp_pos_y_curs = pos_y_curs;

            i = 0; // Счетчик страниц.

            while (tmp_pos_y_curs > 0)
            {
                tmp_pos_y_curs -= PAGE_MENU;
                i++;
            }
            tmp_pos_y_curs += PAGE_MENU;

            j = PAGE_MENU; // Страница меню.

            while (i-- > 1)
            {
                while (j--)
                {
                    temp_menu = (menu_item __flash *)(temp_menu->Next); // Следующий пункт меню.
                }
                j = PAGE_MENU; // Страница меню.
            }
        }

        uint8_t pos_y_text_item = 2; //
        j = PAGE_MENU;           // Страница меню.

        while (j--)
        {
            Print_Buf(pos_y_text_item, 2, Get_Addr_Lang_Text()); // вывод названия пункта меню.

            temp_menu = (menu_item __flash *)(temp_menu->Next); // Следующий пункт меню.

            if ((void *)temp_menu == (void *)&NULL_ENTRY) // Если элемент Next
                return;                                   // пустой, то выход.
            else
                pos_y_text_item++;
        }
    }
}
//==================

//==================
void inc_pos_y_curs(void)
{
    if (quant_items > 1)
    {
        if (pos_y_curs < quant_items)
            pos_y_curs++;
    }
}

void dec_pos_y_curs(void)
{
    if (quant_items > 1)
    {
        if (pos_y_curs > 1)
            pos_y_curs--;
    }
}
//==================

//==================
void set_pos_curs(void)
{
    if (quant_items > 1)
    {
        signed char tmp = pos_y_curs;

        while (tmp > 0)
        {
            tmp -= PAGE_MENU;
        }

        if (tmp <= 0)
            tmp += PAGE_MENU;

        Print_Char(tmp + 1, 1, ARROW_RIGHT);
    }
}
//==================

//==================
bool chk_change_menu(menu_item __flash *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return false;
    else
        return true;
}
//==================

//==================
char __flash *Get_Addr_Lang_Text(void)
{
    switch (get_lang())
    {
    case RUS:
        return temp_menu->Rus_Text;

    case ANGL:
        return temp_menu->Angl_Text;

    default:
        return NULL_TEXT;
    }
}
//==================