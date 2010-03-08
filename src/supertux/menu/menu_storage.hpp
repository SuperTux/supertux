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

class JoystickMenu;
class KeyboardMenu;
class Menu;
class OptionsMenu;
class ProfileMenu;

class MenuStorage
{
public:
  MenuStorage();

  static OptionsMenu*  get_options_menu();
  static ProfileMenu*  get_profile_menu();
  static KeyboardMenu* get_key_options_menu();
  static JoystickMenu* get_joystick_options_menu();

private:
  static OptionsMenu*  options_menu;
  static ProfileMenu*  profile_menu;
  static KeyboardMenu* key_options_menu;
  static JoystickMenu* joystick_options_menu;

private:
  MenuStorage(const MenuStorage&);
  MenuStorage& operator=(const MenuStorage&);
};

#endif

/* EOF */
