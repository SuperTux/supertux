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

#ifndef HEADER_SUPERTUX_CONTROL_JOYSTICK_MENU_HPP
#define HEADER_SUPERTUX_CONTROL_JOYSTICK_MENU_HPP

#include "control/joystickkeyboardcontroller.hpp"
#include "control/controller.hpp"
#include "gui/menu.hpp"
#include "gui/menu_item.hpp"

class JoystickMenu : public Menu
{
public:
  JoystickMenu(JoystickKeyboardController* controller);
  virtual ~JoystickMenu();

  void update();
  std::string get_button_name(int button);
  void update_menu_item(Controller::Control id);
  virtual void menu_action(MenuItem* item);
  JoystickKeyboardController* controller;

private:
  void recreateMenu();

private:
  JoystickMenu(const JoystickMenu&);
  JoystickMenu& operator=(const JoystickMenu&);
};

#endif

/* EOF */
