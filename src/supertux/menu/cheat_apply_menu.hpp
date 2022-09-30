//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_CHEAT_APPLY_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_CHEAT_APPLY_MENU_HPP

#include "gui/menu.hpp"

class Player;

class CheatApplyMenu final : public Menu
{
public:
  CheatApplyMenu(std::function<void(Player&)> callback);
  /** Use this for cheats that need a stack count, e. g. giving fire flowers */
  CheatApplyMenu(std::function<void(Player&, int)> callback);

  void menu_action(MenuItem& item) override;

private:
  std::function<void(Player&)> m_callback_1;
  std::function<void(Player&, int)> m_callback_2;
  int m_stack_count;

private:
  CheatApplyMenu(const CheatApplyMenu&) = delete;
  CheatApplyMenu& operator=(const CheatApplyMenu&) = delete;
};

#endif

/* EOF */
