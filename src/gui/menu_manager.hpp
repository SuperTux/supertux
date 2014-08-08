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
class DrawingContext;

class MenuManager
{
private:
  static MenuManager* s_instance;
public:
  static MenuManager& instance();

public:
  std::vector<Menu*> m_last_menus;
  std::list<Menu*> m_all_menus;

  /** Used only for transition effects */
  Menu* m_previous;

  Menu* m_current;

  friend class Menu;

public:
  MenuManager();
  ~MenuManager();

  void draw(DrawingContext& context);

  /** Set the current menu, if pmenu is NULL, hide the current menu */
  void set_current(Menu* pmenu);

  void push_current(Menu* pmenu);
  void pop_current();

  void recalc_pos();

  /** Return the current active menu or NULL if none is active */
  Menu* current() const
  {
    return m_current;
  }

  bool is_active() const
  {
    return m_current != nullptr;
  }

private:
  MenuManager(const MenuManager&);
  MenuManager& operator=(const MenuManager&);
};

#endif

/* EOF */
