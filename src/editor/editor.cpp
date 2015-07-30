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

#include "editor/editor.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"

Editor::Editor() :
  levelset(),
  levelfile(),
  quit_request(false),
  newlevel_request(false)
{
}

void
Editor::launch() {
  if (levelset == "") {
    MenuManager::instance().set_menu(MenuStorage::EDITOR_LEVELSET_SELECT_MENU);
  }
//  GameSession::current() = this;
}
