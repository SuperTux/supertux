//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_MENU_H
#define SUPERTUX_MENU_H

#include <SDL.h>
#include <vector>
#include <stack>
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
  static std::stack<Menu*> last_menus;
  static Menu* current_;
public:
  /** Set the current menu, if pmenu is NULL, hide the current menu */
  static void set_current(Menu* pmenu);

  /** Return the current active menu or NULL if none is active */
  static Menu* current() { return current_; }

private:
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

  // position of the menu (ie. center of the menu, not top/left)
  int pos_x;
  int pos_y;
  bool has_backitem;

  /** input event for the menu (up, down, left, right, etc.) */
  MenuAction menuaction;

  /* input implementation variables */
  int delete_character;
  char mn_input_char;
  
  int width();
  int height();

public:
  Timer effect;
  int arrange_left;
  int active_item;

  std::vector<MenuItem> item;

  Menu();
  ~Menu();

  void additem(MenuItem* pmenu_item);
  void additem(MenuItemKind kind, const std::string& text, int init_toggle, Menu* target_menu);
  void action ();
  
  /** Remove all entries from the menu */
  void clear();

  /** Check, if the value of the active menu item has changed. FIXME:
      Somebody should document the exact meaning of this function a
      bit more */
  int  check  ();

  void draw   ();
  void draw_item(int index, int menu_width, int menu_height);
  void set_pos(int x, int y, float rw = 0, float rh = 0);

  /** translate a SDL_Event into a menu_action */
  void event(SDL_Event& event);
};

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

#endif /*SUPERTUX_MENU_H*/

/* Local Variables: */
/* mode: c++ */
/* End: */
