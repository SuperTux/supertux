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

class MenuManager
{
private:
  static MenuManager* s_instance;
public:
  static MenuManager& instance();

public:
  std::vector<Menu*> m_last_menus;
  std::vector<Menu*> m_all_menus;

  Menu* m_previous;
  Menu* m_current;

public:
  MenuManager();
  ~MenuManager();

  /** Set the current menu, if pmenu is NULL, hide the current menu */
  void set_current(Menu* pmenu);

  void push_current(Menu* pmenu);
  void pop_current();

  void recalc_pos();

  Menu* get_previous()
  {
    return m_previous;
  }

  /** Return the current active menu or NULL if none is active */
  Menu* current()
  {
    return m_current;
  }


  /** Return the current active menu or NULL if none is active */
  void current(Menu* menu)
  {
    m_current = menu;
  }

private:
  MenuManager(const MenuManager&);
  MenuManager& operator=(const MenuManager&);
};

#endif

/* EOF */
