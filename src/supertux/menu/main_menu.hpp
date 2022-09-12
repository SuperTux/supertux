//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

enum MainMenuIDs {
  MNID_STARTGAME,
  MNID_ADDONS,
  MNID_MANAGEASSETS,
  MNID_OPTIONMENU,
  MNID_LEVELEDITOR,
  MNID_CREDITS,
  MNID_DONATE,
  MNID_QUITMAINMENU
};

class MainMenu final : public Menu
{
public:
  MainMenu();

  void on_window_resize() override;
  void menu_action(MenuItem& item) override;
  bool on_back_action() override { return false; }

private:
  MainMenu(const MainMenu&) = delete;
  MainMenu& operator=(const MainMenu&) = delete;
};

#endif

/* EOF */
