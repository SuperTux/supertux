/*
  menu.h
  
  Super Tux - Menu
  
  by Tobias Glaesser
  tobi.web@gmx.de
  http://www.newbreedsoftware.com/supertux/
  
  December 20, 2003
*/

#ifndef SUPERTUX_MENU_H
#define SUPERTUX_MENU_H

#include <SDL.h>
#include "texture.h"
#include "timer.h"
#include "type.h"
#include "mousecursor.h"

/* Kinds of menu items */
enum MenuItemKind {
  MN_ACTION,
  MN_GOTO,
  MN_TOGGLE,
  MN_BACK,
  MN_DEACTIVE,
  MN_TEXTFIELD,
  MN_NUMFIELD,
  MN_CONTROLFIELD,
  MN_STRINGSELECT,
  MN_LABEL,
  MN_HL, /* horizontal line */
};

class Menu;

struct menu_item_type
  {
    MenuItemKind kind;
    int toggled;
    char *text;
    char *input;
    string_list_type* list;
    Menu* target_menu;
};

menu_item_type* menu_item_create(MenuItemKind kind, char *text, int init_toggle, Menu* target_menu);
void menu_item_change_text (menu_item_type* pmenu_item, const char *text);
void menu_item_change_input(menu_item_type* pmenu_item, const char *text);

class Menu
{
friend void menu_event(SDL_Event& event);

private:
  // position of the menu (ie. center of the menu, not top/left)
  int pos_x;
  int pos_y;
  
  int num_items;
  Menu* last_menu;
  int width();
  int height();
  
public:
  Timer effect;
  int arrange_left;
  int active_item;
  menu_item_type *item;

  static void set_current(Menu* pmenu);

  Menu();
  ~Menu();

  void additem(menu_item_type* pmenu_item);
  void additem(MenuItemKind kind, char *text, int init_toggle, Menu* target_menu);
  void action ();

  /** Check, if the value of the active menu item has changed. */
  int  check  ();
  void draw   ();
  void draw_item(int index, int menu_width, int menu_height);
  void set_pos(int x, int y, float rw = 0, float rh = 0);
};


/* Action done on the menu */
enum MenuAction {
  MENU_ACTION_NONE = -1,
  MENU_ACTION_UP,
  MENU_ACTION_DOWN,
  MENU_ACTION_LEFT,
  MENU_ACTION_RIGHT,
  MENU_ACTION_HIT,
  MENU_ACTION_INPUT,
  MENU_ACTION_REMOVE
};

/* (global) menu variables */
extern MenuAction menuaction;
extern bool show_menu;
extern bool menu_change;
extern texture_type checkbox, checkbox_checked, back, arrow_left, arrow_right;

extern Menu* contrib_menu;
extern Menu* main_menu;
extern Menu* game_menu;
extern Menu* options_menu;
extern Menu* options_controls_menu;
extern Menu* highscore_menu;
extern Menu* load_game_menu;
extern Menu* save_game_menu;
extern Menu* current_menu;

/* input implementation variables */
extern int delete_character;
extern char mn_input_char;

/* Reset the global menu variables */
void menu_reset(void);

/* "Calculate" and draw the menu */
void menu_process_current(void);

/* Check for a menu event */
void menu_event(SDL_Event& event);

#endif /*SUPERTUX_MENU_H*/

/* Local Variables: */
/* mode:c++ */
/* End */
