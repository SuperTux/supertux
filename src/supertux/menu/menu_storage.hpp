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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_MENU_STORAGE_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_MENU_STORAGE_HPP

#include <memory>

class JoystickMenu;
class KeyboardMenu;
class Menu;
class OptionsMenu;
class ProfileMenu;

class MenuStorage
{
private:
  static MenuStorage* s_instance;
public:
  static MenuStorage& instance();

public:
  MenuStorage();
  ~MenuStorage();

  OptionsMenu*  get_options_menu();
  ProfileMenu*  get_profile_menu();
  KeyboardMenu* get_key_options_menu();
  JoystickMenu* get_joystick_options_menu();

private:
  std::unique_ptr<OptionsMenu>  m_options_menu;
  std::unique_ptr<ProfileMenu>  m_profile_menu;
  std::unique_ptr<KeyboardMenu> m_key_options_menu;
  std::unique_ptr<JoystickMenu> m_joystick_options_menu;

private:
  MenuStorage(const MenuStorage&);
  MenuStorage& operator=(const MenuStorage&);
};

#endif

/* EOF */
