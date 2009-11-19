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

Menu* MenuStorage::options_menu = 0;
Menu* MenuStorage::profile_menu = 0;

Menu*
MenuStorage::get_options_menu()
{
  options_menu = new OptionsMenu();
  return options_menu;
}

void
MenuStorage::free_options_menu()
{
  delete options_menu;
  options_menu = 0;
}

Menu*
MenuStorage::get_profile_menu()
{
  profile_menu = new ProfileMenu();
  return profile_menu;
}

void
MenuStorage::free_profile_menu()
{
  delete profile_menu;
  profile_menu = 0;
}

/* EOF */
