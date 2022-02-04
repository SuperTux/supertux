//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_GUI_MENU_BADGUY_SELECT_HPP
#define HEADER_SUPERTUX_GUI_MENU_BADGUY_SELECT_HPP

#include "gui/menu.hpp"

class BadguySelectMenu final : public Menu
{
public:
  BadguySelectMenu(std::vector<std::string>* badguys_);

  void menu_action(MenuItem& item) override;

  static std::vector<std::string> all_badguys;

  void remove_badguy();

  void refresh() override;

private:
  std::vector<std::string>* badguys;
  int selected;
  int remove_item;

  void add_badguy();

private:
  BadguySelectMenu(const BadguySelectMenu&) = delete;
  BadguySelectMenu& operator=(const BadguySelectMenu&) = delete;
};

#endif

/* EOF */
