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
class HorizontalMenu;
class Menu;
class MenuTransition;
class Notification;
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

  std::unique_ptr<Notification> m_notification;
  bool m_has_next_notification;
  std::unique_ptr<Notification> m_next_notification;

  std::unique_ptr<HorizontalMenu> m_horizontal_menu;
  bool m_has_next_horizontal_menu;
  std::unique_ptr<HorizontalMenu> m_next_horizontal_menu;

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
  void set_notification(std::unique_ptr<Notification> notification);
  void set_horizontal_menu(std::unique_ptr<HorizontalMenu> horizontal_menu);

  void set_menu(int id);
  void set_menu(std::unique_ptr<Menu> menu);
  void push_menu(int id, bool skip_transition = false);
  void push_menu(std::unique_ptr<Menu> menu, bool skip_transition = false);
  void pop_menu(bool skip_transition = false);
  void clear_menu_stack();

  void on_window_resize();
  bool is_active() const
  {
    return !m_menu_stack.empty() || m_horizontal_menu;
  }

  bool has_dialog() const
  {
    return m_dialog || m_has_next_dialog;
  }
  Menu* current_menu() const;
  Menu* previous_menu() const;

private:
  void transition(Menu* from, Menu* to, bool call_this = false); // "call_this" -> calls this specific overload to prevent ambiguous calls
  void transition(Menu* from, Dialog* to);
  void transition(Dialog* from, Menu* to);
  void transition(Dialog* from, Dialog* to);

private:
  MenuManager(const MenuManager&) = delete;
  MenuManager& operator=(const MenuManager&) = delete;
};

#endif

/* EOF */
