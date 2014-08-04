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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_MAIN_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_MAIN_MENU_HPP

#include "gui/menu.hpp"

class AddonMenu;
class ContribMenu;
class World;

enum MainMenuIDs {
  MNID_STARTGAME,
  MNID_LEVELS_CONTRIB,
  MNID_ADDONS,
  MNID_OPTIONMENU,
  MNID_LEVELEDITOR,
  MNID_CREDITS,
  MNID_QUITMAINMENU
};

class MainMenu : public Menu
{
private:
  std::unique_ptr<AddonMenu>   m_addon_menu;
  std::unique_ptr<ContribMenu> m_contrib_menu;
  std::unique_ptr<World>       m_main_world;

public:
  MainMenu();

  void check_menu();

private:
  MainMenu(const MainMenu&);
  MainMenu& operator=(const MainMenu&);
};

#endif

/* EOF */
