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
#include "editor/spawnpoint_marker.hpp"
#include "gui/menu_item.hpp"
#include "object/background.hpp"
#include "object/camera.hpp"
#include "object/tilemap.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile_set.hpp"
#include "supertux/world.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/gettext.hpp"

const std::string DEFAULT_BG_TOP    = "images/background/BlueRock_Forest/blue-top.jpg";
const std::string DEFAULT_BG_MIDDLE = "images/background/BlueRock_Forest/blue-middle.jpg";
const std::string DEFAULT_BG_BOTTOM = "images/background/BlueRock_Forest/blue-bottom.jpg";

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
  TileSet* tileset = Editor::current()->get_tileset();
  std::unique_ptr<Sector> new_sector = std::unique_ptr<Sector>(new Sector(level));

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

  auto background = std::make_shared<Background>();
  background->set_images(DEFAULT_BG_TOP, DEFAULT_BG_MIDDLE, DEFAULT_BG_BOTTOM);
  background->set_speed(0.5);
  new_sector->add_object(background);

  auto bkgrd = std::make_shared<TileMap>(tileset);
  bkgrd->resize(100, 35);
  bkgrd->set_layer(-100);
  bkgrd->set_solid(false);
  new_sector->add_object(bkgrd);

  auto intact = std::make_shared<TileMap>(tileset);
  intact->resize(100, 35);
  intact->set_layer(0);
  intact->set_solid(true);
  new_sector->add_object(intact);

  auto frgrd = std::make_shared<TileMap>(tileset);
  frgrd->resize(100, 35);
  frgrd->set_layer(100);
  frgrd->set_solid(false);
  new_sector->add_object(frgrd);

  auto spawn_point = std::make_shared<SpawnPoint>();
  spawn_point->name = "main";
  spawn_point->pos = Vector(64, 480);
  new_sector->spawnpoints.push_back(spawn_point);

  GameObjectPtr spawn_point_marker = std::make_shared<SpawnPointMarker>( spawn_point.get() );
  new_sector->add_object(spawn_point_marker);

  auto camera = std::make_shared<Camera>(new_sector.get(), "Camera");
  new_sector->add_object(camera);

  new_sector->update_game_objects();

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
