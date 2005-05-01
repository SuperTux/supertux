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

#include <vector>
#include <set>
#include <string>
#include <utility>

#include "SDL.h"

#include "video/surface.h"
#include "video/font.h"
#include "special/timer.h"
#include "mousecursor.h"

bool confirm_dialog(Surface* background, std::string text);

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
  MenuItem(MenuItemKind kind, int id = -1);
  MenuItemKind kind;
  int id;   // item id      
  bool toggled;
  std::string text;
  std::string input;
  
  std::vector<std::string> list; // list of values for a STRINGSELECT item
  size_t selected; // currently selected item
  
  Menu* target_menu;
  
  void change_text (const std::string& text);
  void change_input(const std::string& text);
  
  static MenuItem* create(MenuItemKind kind, const std::string& text,
                          int init_toggle, Menu* target_menu, int id, int key);
  
  std::string get_input_with_symbol(bool active_item);   // returns the text with an input symbol

private:
  /// copy-construction not allowed
  MenuItem(const MenuItem& other) { assert(false); }
  /// assignment not allowed
  void operator= (const MenuItem& other) { assert(false); }

  /// keyboard key or joystick button
  bool input_flickering;
  Timer input_flickering_timer;
};

class Menu
{
private:
  static std::vector<Menu*> last_menus;
  static Menu* current_;
  
  static void push_current(Menu* pmenu);
  static void pop_current();
  
public:
  /** Set the current menu, if pmenu is NULL, hide the current menu */
  static void set_current(Menu* pmenu);
  
  /** Return the current active menu or NULL if none is active */
  static Menu* current()
  {
    return current_;
  }
  
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
    MENU_ACTION_REMOVE,
    MENU_ACTION_BACK
  };
  
  /** Number of the item that got 'hit' (ie. pressed) in the last
      event()/action() call, -1 if none */
  int hit_item;
  
  // position of the menu (ie. center of the menu, not top/left)
  int pos_x;
  int pos_y;
  
  /** input event for the menu (up, down, left, right, etc.) */
  MenuAction menuaction;
  
  /* input implementation variables */
  int delete_character;
  char mn_input_char;
  Timer repeat_timer;

public:
  static Font* default_font;
  static Font* active_font;
  static Font* deactive_font;
  static Font* label_font;
  static Font* field_font;

  std::vector<MenuItem*> items;
  
  Menu();
  ~Menu();
  
  void add_hl();
  void add_label(const std::string& text);
  void add_entry(int id, const std::string& text);
  void add_toggle(int id, const std::string& text, bool toggled = false);
  void add_deactive(int id, const std::string& text);
  void add_back(const std::string& text);
  void add_submenu(const std::string& text, Menu* submenu, int id = -1);
  void add_controlfield(int id, const std::string& text,
			const std::string& mapping = "");

  virtual void menu_action(MenuItem* item);
  
  void action();
  
  /** Remove all entries from the menu */
  void clear();
  
  /** Return the index of the menu item that was 'hit' (ie. the user
      clicked on it) in the last event() call */
  int check ();
  
  MenuItem& get_item(int index)
  {
    return *(items[index]);
  }
  MenuItem& get_item_by_id(int id);
  const MenuItem& get_item_by_id(int id) const;
  
  int get_active_item_id();
  void set_active_item(int id);
  
  void draw(DrawingContext& context);  
  void set_pos(int x, int y, float rw = 0, float rh = 0);
  
  void event(const SDL_Event& event);

  bool is_toggled(int id) const;

protected:
  void additem(MenuItem* pmenu_item);  
  int get_width() const;
  int get_height() const;

private:
  void check_controlfield_change_event(const SDL_Event& event);  
  void draw_item(DrawingContext& context, int index);
  Timer effect;
  int arrange_left;
  int active_item;
};

extern Surface* checkbox;
extern Surface* checkbox_checked;
extern Surface* back;
extern Surface* arrow_left;
extern Surface* arrow_right;

#endif
