//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_GUI_MENU_MANAGER_HPP
#define HEADER_SUPERTUX_GUI_MENU_MANAGER_HPP

#include <vector>
#include <list>

class Menu;

class MenuManager2 // FIXME: temporary name 
{
public:
  static std::vector<Menu*> last_menus;
  static Menu* previous;
  static Menu* current_;

public:
  /** Pointers to all currently available menus, used to handle repositioning on window resize */
  static std::list<Menu*>   all_menus;

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
  MenuManager2(const MenuManager2&);
  MenuManager2& operator=(const MenuManager2&);
};

#endif

/* EOF */
