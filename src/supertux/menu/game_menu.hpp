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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_GAME_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_GAME_MENU_HPP

#include "gui/menu.hpp"

class Level;

enum GameMenuIDs {
  MNID_CONTINUE,
  MNID_ABORTLEVEL
};

class GameMenu : public Menu
{
private:
public:
  GameMenu(const Level& level);

  void check_menu();

private:
  GameMenu(const GameMenu&);
  GameMenu& operator=(const GameMenu&);
};

#endif

/* EOF */
