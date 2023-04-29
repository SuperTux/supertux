//  SuperTux
//  Copyright (C) 2023 Vankata453
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

#ifndef HEADER_SUPERTUX_GUI_MENU_BASE_HPP
#define HEADER_SUPERTUX_GUI_MENU_BASE_HPP

#include "math/vector.hpp"

#include "gui/menu_action.hpp"

class Controller;
class DrawingContext;
union SDL_Event;

namespace Base {

/** Base class for all GUI menus, taking place in the MenuManager menu stack. */
class Menu
{
public:
  Menu() {}
  virtual ~Menu() {}

  virtual void refresh() {}

  virtual void draw(DrawingContext& context) = 0;

  virtual void process_action(const MenuAction& action) = 0;
  virtual void event(const SDL_Event& ev) = 0;

  virtual void on_window_resize() = 0;

  // Get properties
  virtual float get_width() const = 0;
  virtual float get_height() const = 0;
  virtual Vector get_center_pos() const = 0;

  /** Returns true, when the text is more important than the action. */
  virtual bool is_sensitive() const { return false; }
};

} // namespace Base

#endif

/* EOF */
