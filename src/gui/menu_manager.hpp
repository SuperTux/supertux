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
#include <memory>

#include "SDL.h"

class DrawingContext;
class Menu;
class MenuTransition;

class MenuManager
{
private:
  static MenuManager* s_instance;
public:
  static MenuManager& instance();

public:
  std::vector<std::unique_ptr<Menu> > m_menu_stack;
  std::unique_ptr<MenuTransition> m_transition;

  friend class Menu;

public:
  MenuManager();
  ~MenuManager();

  void event(const SDL_Event& event);
  void update();
  void refresh();

  void draw(DrawingContext& context);
  bool check_menu();

  void set_menu(int id);
  void set_menu(std::unique_ptr<Menu> menu);
  void push_menu(int id);
  void push_menu(std::unique_ptr<Menu> menu);
  void pop_menu();
  void clear_menu_stack();

  void recalc_pos();
  bool is_active() const
  {
    return !m_menu_stack.empty();
  }

private:
  Menu* current() const;
  void transition(Menu* from, Menu* to);

private:
  MenuManager(const MenuManager&);
  MenuManager& operator=(const MenuManager&);
};

#endif

/* EOF */
