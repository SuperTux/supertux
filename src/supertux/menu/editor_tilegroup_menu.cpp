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

#include "supertux/menu/editor_tilegroup_menu.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
//#include "supertux/levelset.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/title_screen.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorTilegroupMenu::EditorTilegroupMenu()
{
  add_label(_("Tilegroups"));
  add_hl();

  int id = 0;
  for(auto i = Editor::current()->level->tileset->tilegroups.begin(); i != Editor::current()->level->tileset->tilegroups.end(); ++i) {
    Tilegroup* tg = &(*i);
    add_entry(id, tg->name);
    id++;
  }

  add_hl();
  add_entry(-1,_("Abort"));
}

void
EditorTilegroupMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    //Editor::current()->tileselect.tilegroup_ID = item->id;
    Editor::current()->tileselect.active_tilegroup = Editor::current()->level->tileset->tilegroups[item->id].tiles;
    Editor::current()->reactivate_request = true;
    //Editor::current()->reload_request = true;
    Editor::current()->tileselect.input_type = EditorInputGui::IP_TILE;
    MenuManager::instance().clear_menu_stack();
  }else{
    MenuManager::instance().clear_menu_stack();
    Editor::current()->reactivate_request = true;
  }
}

/* EOF */
