//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#include "gui/item_goto.hpp"

#include <stdio.h>

#include "gui/menu.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"

ItemGoTo::ItemGoTo(const std::string& text_, int target_menu_, int _id) :
  MenuItem(text_, _id),
  target_menu(target_menu_)
{
}

void
ItemGoTo::process_action(const MenuAction& action) {
  if (action == MENU_ACTION_HIT) {
    assert(target_menu != 0);
    MenuManager::instance().push_menu(target_menu);
  }
}

/* EOF */
