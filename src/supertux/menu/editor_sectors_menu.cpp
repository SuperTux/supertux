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

#include "supertux/menu/editor_sectors_menu.hpp"

#include <sstream>

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "gui/menu_item.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/sector_parser.hpp"
#include "util/file_system.hpp"
#include "util/gettext.hpp"

EditorSectorsMenu::EditorSectorsMenu()
{
  add_label(_("Choose sector to edit:"));
  add_hl();

  int id = 0;
  for(auto i = Editor::current()->get_level()->sectors.begin();
      i != Editor::current()->get_level()->sectors.end(); ++i) {
    add_entry(id, (*i)->get_name());
    id++;
  }

  add_hl();
  add_submenu(_("Sector settings..."), MenuStorage::EDITOR_SECTOR_MENU);
  add_entry(-2,_("Create new sector"));
  add_entry(-3,_("Abort"));
}

void
EditorSectorsMenu::create_sector()
{
  Level* level = Editor::current()->get_level();

  auto new_sector = SectorParser::from_nothing(*level);

  if (!new_sector) {
    log_warning << "Failed to create a new sector." << std::endl;
    return;
  }

  // Find an unique name
  std::string sector_name;
  int num = 2;
  do {
    sector_name = "sector" + std::to_string(num);
    num++;
  } while ( level->get_sector(sector_name) );
  *(new_sector->get_name_ptr()) = sector_name;

  level->add_sector(move(new_sector));
  Editor::current()->load_sector(level->get_sector_count() - 1);
  MenuManager::instance().clear_menu_stack();
  Editor::current()->reactivate_request = true;
}

void
EditorSectorsMenu::menu_action(MenuItem* item)
{
  if (item->id >= 0)
  {
    Editor::current()->load_sector(item->id);
    Editor::current()->reactivate_request = true;
    MenuManager::instance().clear_menu_stack();
  } else {
    switch (item->id) {
      case -1:
        break;
      case -2:
        create_sector();
        break;
      case -3:
        MenuManager::instance().clear_menu_stack();
        Editor::current()->reactivate_request = true;
        break;
    }
  }
}

/* EOF */
