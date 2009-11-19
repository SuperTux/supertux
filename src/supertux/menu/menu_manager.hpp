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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_MENU_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_MENU_MANAGER_HPP

class Menu;

class MenuManager
{
public:
  MenuManager();

  static Menu* get_options_menu();
  static void free_options_menu();

  static Menu* get_profile_menu();
  static void free_profile_menu();

private:
  static Menu* options_menu;
  static Menu* profile_menu;

private:
  MenuManager(const MenuManager&);
  MenuManager& operator=(const MenuManager&);
};

#endif

/* EOF */
