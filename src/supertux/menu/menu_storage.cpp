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

#include "supertux/menu/menu_storage.hpp"

#include "supertux/menu/options_menu.hpp"
#include "supertux/menu/profile_menu.hpp"
#include "supertux/menu/joystick_menu.hpp"
#include "supertux/menu/keyboard_menu.hpp"
#include "supertux/globals.hpp"

OptionsMenu*  MenuStorage::options_menu = 0;
ProfileMenu*  MenuStorage::profile_menu = 0;
KeyboardMenu* MenuStorage::key_options_menu = 0;
JoystickMenu* MenuStorage::joystick_options_menu = 0;

OptionsMenu*
MenuStorage::get_options_menu()
{
  options_menu = new OptionsMenu();
  return options_menu;
}

ProfileMenu*
MenuStorage::get_profile_menu()
{
  profile_menu = new ProfileMenu();
  return profile_menu;
}

KeyboardMenu*
MenuStorage::get_key_options_menu()
{
  if (!key_options_menu)
  { // FIXME: this in never freed
    key_options_menu = new KeyboardMenu(g_jk_controller);
  }

  return key_options_menu;
}

JoystickMenu*
MenuStorage::get_joystick_options_menu()
{
  if (!joystick_options_menu)
  { // FIXME: this in never freed
    joystick_options_menu = new JoystickMenu(g_jk_controller);
  }

  return joystick_options_menu;
}

/* EOF */
