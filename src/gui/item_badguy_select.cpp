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

#include "gui/item_badguy_select.hpp"

#include <stdio.h>

#include "gui/menu.hpp"
#include "gui/menu_badguy_select.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"

ItemBadguySelect::ItemBadguySelect(const std::string& text_, std::vector<std::string>* badguys_, int _id) :
  MenuItem(text_, _id),
  badguys(badguys_)
{
}

void
ItemBadguySelect::process_action(const MenuAction& action) {
  if (action == MENU_ACTION_HIT) {
    MenuManager::instance().push_menu(std::unique_ptr<Menu>(new BadguySelectMenu(badguys)));
  }
}

/* EOF */
