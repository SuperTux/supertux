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
#include <vector>
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

class MenuItem
{
public:
  MenuItemKind kind;
  int toggled;
  char *text;
  char *input;
  string_list_type* list;
  Menu* target_menu;

  void change_text (const char *text);
  void change_input(const char *text);

  static MenuItem* create(MenuItemKind kind, const char *text, int init_toggle, Menu* target_menu);
};

class Menu
{
private:
  // position of the menu (ie. center of the menu, not top/left)
  int pos_x;
  int pos_y;
  
  Menu* last_menu;
  int width();
  int height();
  
public:
  Timer effect;
  int arrange_left;
  int active_item;
  std::vector<MenuItem> item;

  static void set_current(Menu* pmenu);

  Menu();
  ~Menu();

  void additem(MenuItem* pmenu_item);
  void additem(MenuItemKind kind, const std::string& text, int init_toggle, Menu* target_menu);
  void action ();
  
  /** Remove all entries from the menu */
  void clear();

  /** Check, if the value of the active menu item has changed. */
  int  check  ();
  void draw   ();
  void draw_item(int index, int menu_width, int menu_height);
  void set_pos(int x, int y, float rw = 0, float rh = 0);

  /* Check for a menu event */
  void event(SDL_Event& event);
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

extern Surface* checkbox;
extern Surface* checkbox_checked;
extern Surface* back;
extern Surface* arrow_left;
extern Surface* arrow_right;

extern Menu* contrib_menu;
extern Menu* contrib_subset_menu;
extern Menu* main_menu;
extern Menu* game_menu;
extern Menu* worldmap_menu;
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

#endif /*SUPERTUX_MENU_H*/

/* Local Variables: */
/* mode: c++ */
/* End */
