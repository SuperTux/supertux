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

#include "supertux/menu/editor_new_levelset_menu.hpp"

#include <physfs.h>
#include <sstream>

#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/menu/contrib_levelset_menu.hpp"
#include "supertux/menu/editor_levelset_select_menu.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorNewLevelsetMenu::EditorNewLevelsetMenu() :
  levelset_name(),
  levelset_desc()
{
  add_label(_("New level subset"));
  add_hl();

  add_textfield(_("Name"), &levelset_name);
  add_textfield(_("Description"), &levelset_desc);

  add_entry(1,_("OK"));

  add_hl();
  add_back(_("Back"));
}

void
EditorNewLevelsetMenu::menu_action(MenuItem* item)
{
  if (item->id > 0)
  {
    std::unique_ptr<World> new_world = World::create(levelset_name, levelset_desc);
    new_world->save();
    Editor::current()->world = move(new_world);

    MenuManager::instance().pop_menu();
    MenuManager::instance().push_menu(MenuStorage::EDITOR_LEVEL_SELECT_MENU);
  }
}

/* EOF */
