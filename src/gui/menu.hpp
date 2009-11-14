//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#include <list>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <assert.h>

#include <SDL.h>

#include "video/surface.hpp"
#include "video/font.hpp"
#include "mousecursor.hpp"

bool confirm_dialog(Surface* background, std::string text);

/* Kinds of menu items */
enum MenuItemKind {
  MN_ACTION,
  MN_GOTO,
  MN_TOGGLE,
  MN_BACK,
  MN_INACTIVE,
  MN_TEXTFIELD,
  MN_NUMFIELD,
  MN_CONTROLFIELD,
  MN_STRINGSELECT,
  MN_LABEL,
  MN_HL /* horizontal line */
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
  std::string help;

  std::vector<std::string> list; // list of values for a STRINGSELECT item
  size_t selected; // currently selected item

  Menu* target_menu;

  void set_help(const std::string& help_text);

  void change_text (const std::string& text);
  void change_input(const std::string& text);

  static MenuItem* create(MenuItemKind kind, const std::string& text,
                          int init_toggle, Menu* target_menu, int id, int key);

  std::string get_input_with_symbol(bool active_item);   // returns the text with an input symbol

private:
  /// copy-construction not allowed
  MenuItem(const MenuItem& ) { assert(false); }
  /// assignment not allowed
  void operator= (const MenuItem& ) { assert(false); }

  /// keyboard key or joystick button
  bool input_flickering;
};

class Menu
{
  static Color default_color;
  static Color active_color;
  static Color inactive_color;
  static Color label_color;
  static Color field_color;
private:
  static std::vector<Menu*> last_menus;

  /** Pointers to all currently available menus, used to handle repositioning on window resize */
  static std::list<Menu*>   all_menus;

  static Menu* previous;
  static Menu* current_;

public:
  /** Set the current menu, if pmenu is NULL, hide the current menu */
  static void set_current(Menu* pmenu);

  static void push_current(Menu* pmenu);
  static void pop_current();


  static void recalc_pos();

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
      event()/update() call, -1 if none */
  int hit_item;

  // position of the menu (ie. center of the menu, not top/left)
  float pos_x;
  float pos_y;

  /** input event for the menu (up, down, left, right, etc.) */
  MenuAction menuaction;

  /* input implementation variables */
  int   delete_character;
  char  mn_input_char;
  float menu_repeat_time;

  bool close;

public:
  std::vector<MenuItem*> items;

  Menu();
  virtual ~Menu();

  MenuItem* add_hl();
  MenuItem* add_label(const std::string& text);
  MenuItem* add_entry(int id, const std::string& text);
  MenuItem* add_toggle(int id, const std::string& text, bool toggled = false);
  MenuItem* add_inactive(int id, const std::string& text);
  MenuItem* add_back(const std::string& text);
  MenuItem* add_submenu(const std::string& text, Menu* submenu, int id = -1);
  MenuItem* add_controlfield(int id, const std::string& text,
                             const std::string& mapping = "");
  MenuItem* add_string_select(int id, const std::string& text);

  virtual void menu_action(MenuItem* item);

  void update();

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
  void set_pos(float x, float y, float rw = 0, float rh = 0);

  void event(const SDL_Event& event);

  bool is_toggled(int id) const;
  void set_toggled(int id, bool toggled);

  Menu* get_parent() const;

protected:
  void additem(MenuItem* pmenu_item);
  float get_width() const;
  float get_height() const;

private:
  void check_controlfield_change_event(const SDL_Event& event);
  void draw_item(DrawingContext& context, int index);
  float effect_progress;
  float effect_start_time;
  int arrange_left;
  int active_item;

  std::auto_ptr<Surface> checkbox;
  std::auto_ptr<Surface> checkbox_checked;
  std::auto_ptr<Surface> back;
  std::auto_ptr<Surface> arrow_left;
  std::auto_ptr<Surface> arrow_right;
};

#endif
