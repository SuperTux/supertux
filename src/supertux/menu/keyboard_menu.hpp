//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>,
//                2007 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_SUPERTUX_CONTROL_KEYBOARD_MENU_HPP
#define HEADER_SUPERTUX_CONTROL_KEYBOARD_MENU_HPP

#include "control/joystickkeyboardcontroller.hpp"
#include "gui/menu.hpp"
#include "gui/menu_item.hpp"

class KeyboardMenu : public Menu
{
public:
  KeyboardMenu(JoystickKeyboardController* controller);
  ~KeyboardMenu();

  void update();
  std::string get_key_name(SDLKey key);
  virtual void menu_action(MenuItem* item);
  JoystickKeyboardController* controller;

private:
  KeyboardMenu(const KeyboardMenu&);
  KeyboardMenu& operator=(const KeyboardMenu&);
};

#endif

/* EOF */
