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

MenuStorage* MenuStorage::s_instance = 0;

MenuStorage&
MenuStorage::instance()
{
  assert(s_instance);
  return *s_instance;
}

MenuStorage::MenuStorage()
{
  assert(!s_instance);
  s_instance = this;
}

MenuStorage::~MenuStorage()
{
  s_instance = nullptr;
}

OptionsMenu*
MenuStorage::get_options_menu()
{
  if (!m_options_menu)
  {
    m_options_menu.reset(new OptionsMenu);
  }

  return m_options_menu.get();
}

ProfileMenu*
MenuStorage::get_profile_menu()
{
  if (!m_profile_menu)
  {
    m_profile_menu.reset(new ProfileMenu);
  }

  return m_profile_menu.get();
}

KeyboardMenu*
MenuStorage::get_key_options_menu()
{
  if (!m_key_options_menu)
  {
    m_key_options_menu.reset(new KeyboardMenu(g_input_manager));
  }

  return m_key_options_menu.get();
}

JoystickMenu*
MenuStorage::get_joystick_options_menu()
{
  if (!m_joystick_options_menu)
  {
    m_joystick_options_menu.reset(new JoystickMenu(g_input_manager));
  }

  return m_joystick_options_menu.get();
}

/* EOF */
