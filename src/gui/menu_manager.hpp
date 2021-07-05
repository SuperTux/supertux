//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <memory>
#include <vector>

class Controller;
class Dialog;
class DrawingContext;
class Menu;
class MenuTransition;
union SDL_Event;

class MenuManager final
{
private:
  static MenuManager* s_instance;
public:
  static MenuManager& instance();

private:
  std::unique_ptr<Dialog> m_dialog;
  bool m_has_next_dialog;
  std::unique_ptr<Dialog> m_next_dialog;

  std::vector<std::unique_ptr<Menu> > m_menu_stack;
  std::unique_ptr<MenuTransition> m_transition;

public:
  MenuManager();
  ~MenuManager();

  void event(const SDL_Event& event);
  void process_input(const Controller& controller);
  void refresh();

  void draw(DrawingContext& context);

  void set_dialog(std::unique_ptr<Dialog> dialog);

  void set_menu(int id);
  void set_menu(std::unique_ptr<Menu> menu);
  void push_menu(int id);
  void push_menu(std::unique_ptr<Menu> menu);
  void pop_menu();
  void clear_menu_stack();

  void on_window_resize();
  bool is_active() const
  {
    return !m_menu_stack.empty();
  }

  bool has_dialog() const
  {
    return m_dialog || m_has_next_dialog;
  }
  Menu* current_menu() const;

private:
  void transition(Menu* from, Menu* to);

private:
  MenuManager(const MenuManager&) = delete;
  MenuManager& operator=(const MenuManager&) = delete;
};

#endif

/* EOF */
