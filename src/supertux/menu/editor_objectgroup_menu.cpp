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

#include "supertux/menu/editor_objectgroup_menu.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "editor/object_group.hpp"
#include "editor/object_input.hpp"
#include "gui/menu_item.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorObjectgroupMenu::EditorObjectgroupMenu()
{
  bool worldmap = Editor::current()->get_worldmap_mode();

  add_label(_("Objects"));
  add_hl();

  int id = 0;
  for(auto& og : Editor::current()->tileselect.object_input->groups) {
    if (worldmap == og.for_worldmap) {
      add_entry(id, og.name);
    }
    id++;
  }

  add_hl();
  add_entry(-1,_("Cancel"));
}

EditorObjectgroupMenu::~EditorObjectgroupMenu()
{
  auto editor = Editor::current();
  if(editor == NULL) {
    return;
  }
  editor->reactivate_request = true;
}

void
EditorObjectgroupMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    auto tileselect = &(Editor::current()->tileselect);
    tileselect->active_objectgroup = item->id;
    tileselect->input_type = EditorInputGui::IP_OBJECT;
    tileselect->reset_pos();
    tileselect->update_mouse_icon();
  }
  MenuManager::instance().clear_menu_stack();
}

/* EOF */
