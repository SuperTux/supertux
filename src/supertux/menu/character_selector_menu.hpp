//  SuperTux
//  Copyright (C) 2024
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

#pragma once

#include "gui/menu.hpp"
#include <functional>

class CharacterSelectorMenu final : public Menu
{
public:
  /**
   * @param on_select Callback called with character ID (0=Tux, 1=Penny, 2=Larry)
   * @param on_cancel Callback called if user presses BACK or cancels
   */
  CharacterSelectorMenu(
    const std::function<void(int)>& on_select,
    const std::function<void()>& on_cancel);

  void menu_action(MenuItem& item) override {}
  bool on_back_action() override;

private:
  std::function<void(int)> m_on_select;
  std::function<void()> m_on_cancel;

  // Character IDs
  static constexpr int CHARACTER_TUX = 0;
  static constexpr int CHARACTER_PENNY = 1;
  static constexpr int CHARACTER_LARRY = 2;
};
