//==================
#ifndef _DMENU_H_

#define _MENU_H_

#include "menu.h"
#include <stdbool.h>

//==================

//==================
// Typedefs:
typedef void (*FuncPtr)(void);
//==================

//==================
typedef struct menu_item
{
    void *Parent;
    void *Child;
    void *Next;
    void *Prev;
    FuncPtr EnterFunc;
    FuncPtr PlusFunc;
    FuncPtr MinusFunc;
    FuncPtr MenuInitFunc;
    char __flash *Rus_Text;
    char __flash *Angl_Text;
} menu_item;
//==================

// Externs:
//==================
extern menu_item __flash *CurrMenuItem; // Текущий пункт меню.

extern menu_item __flash Null_Menu;
//==================

// Defines and Macros:
//==================
#define NULL_ENTRY Null_Menu
#define NULL_FUNC (void *)0
#define NULL_TEXT ""
#define PAGE_MENU 3
//==================

//==================
#define MAKE_MENU(Name, Parent, Child, Next, Prev, EnterFunc, PlusFunc, MinusFunc, MenuInitFunc, Rus_Text, Angl_Text) \
    extern menu_item __flash Parent;                                                                                  \
    extern menu_item __flash Child;                                                                                   \
    extern menu_item __flash Next;                                                                                    \
    extern menu_item __flash Prev;                                                                                    \
    menu_item __flash Name =                                                                                          \
        {                                                                                                             \
            (menu_item *)&Parent,                                                                                     \
            (menu_item *)&Child,                                                                                      \
            (menu_item *)&Next,                                                                                       \
            (menu_item *)&Prev,                                                                                       \
            EnterFunc,                                                                                                \
            PlusFunc,                                                                                                 \
            MinusFunc,                                                                                                \
            MenuInitFunc,                                                                                             \
            {Rus_Text},                                                                                               \
            {Angl_Text}}
//==================

//==================
#define PARENT *((menu_item __flash *)(CurrMenuItem->Parent))
#define CHILD *((menu_item __flash *)(CurrMenuItem->Child))
#define NEXT *((menu_item __flash *)(CurrMenuItem->Next))
#define PREV *((menu_item __flash *)(CurrMenuItem->Prev))
#define ENTER_FUNC *((FuncPtr)(CurrMenuItem->EnterFunc))
#define MENU_PLUS_FUNC *((FuncPtr)(CurrMenuItem->PlusFunc))
#define MENU_MINUS_FUNC *((FuncPtr)(CurrMenuItem->MinusFunc))
#define MENU_INIT_FUNC *((FuncPtr)(CurrMenuItem->MenuInitFunc))
//==================

//==================
#define SET_MENU_LEVEL(x) \
    Set_Menu_Level(&x)

#define SET_MENU_ITEM(x) \
    Set_Menu_Item(&x)

#define GO_MENU_FUNC(x) \
    MenuFunc((FuncPtr *)&x)

#define EXTERN_MENU(Name) \
    extern menu_item __flash Name;

#define CHK_CHANGE_MENU(x) \
    chk_change_menu(&x)
//==================

//==================
enum
{
    SET_LEVEL = 0,
    SET_NEXT,
    SET_PREV,
};
//==================

// Prototypes:
//==================
bool Set_Menu_Level(menu_item __flash *NewMenu);
bool MenuFunc(FuncPtr *Function);
bool chk_change_menu(menu_item __flash *NewMenu);
//==================

//==================
bool proc_menu_keys(void);
//==================

//==================
void Out_Menu_Items_Init(void);
void Out_Menu_Items(void);
//==================

//==================
void out_name_level(void);
u08 count_chars(char __flash *data);
void make_page_menu(void);
void inc_pos_y_curs(void);
void dec_pos_y_curs(void);
void set_pos_curs(void);
//==================

//==================
char __flash *Get_Addr_Lang_Text(void);
//==================

#endif