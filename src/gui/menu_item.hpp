//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_GUI_MENU_ITEM_HPP
#define HEADER_SUPERTUX_GUI_MENU_ITEM_HPP

#include <list>
#include <memory>
#include <SDL.h>

#include "gui/menu.hpp"

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

class MenuItem
{
public:
  MenuItem(MenuItemKind kind, int id = -1);

  void set_help(const std::string& help_text);

  void change_text (const std::string& text);
  void change_input(const std::string& text);

  static MenuItem* create(MenuItemKind kind, const std::string& text,
                          int init_toggle, Menu* target_menu, int id, int key);

  std::string get_input_with_symbol(bool active_item);   // returns the text with an input symbol

public:
  MenuItemKind kind;
  int id;   // item id
  bool toggled;
  std::string text;
  std::string input;
  std::string help;

  std::vector<std::string> list; // list of values for a STRINGSELECT item
  size_t selected; // currently selected item

  Menu* target_menu;

private:
  /// keyboard key or joystick button
  bool input_flickering;

private:
  MenuItem(const MenuItem&);
  MenuItem& operator=(const MenuItem&);
};

#endif

/* EOF */
