//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/menu/worldmap_cheat_menu.hpp"

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "util/log.hpp"
#include "worldmap/level_tile.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"

WorldmapCheatMenu::WorldmapCheatMenu()
{
  auto worldmap = worldmap::WorldMap::current();
  auto& tux = worldmap->get_singleton_by_type<worldmap::Tux>();

  add_label(_("Cheats"));
  add_hl();
  add_entry(MNID_GROW, _("Bonus: Grow"));
  add_entry(MNID_FIRE, _("Bonus: Fire"));
  add_entry(MNID_ICE, _("Bonus: Ice"));
  add_entry(MNID_AIR, _("Bonus: Air"));
  add_entry(MNID_EARTH, _("Bonus: Earth"));
  add_entry(MNID_SHRINK, _("Bonus: None"));
  add_hl();
  add_entry(MNID_GHOST, (tux.get_ghost_mode() ?
                         _("Leave Ghost Mode") : _("Activate Ghost Mode")));
  add_hl();
  add_entry(MNID_FINISH_LEVEL, _("Finish Level"));
  add_entry(MNID_RESET_LEVEL, _("Reset Level"));
  add_hl();
  add_entry(MNID_FINISH_WORLDMAP, _("Finish Worldmap"));
  add_entry(MNID_RESET_WORLDMAP, _("Reset Worldmap"));
  add_hl();
  add_entry(MNID_MOVE_TO_LEVEL, _("Go to level"));
  add_entry(MNID_MOVE_TO_MAIN, _("Go to main spawnpoint"));
  add_hl();
  add_back(_("Back"));
}

void
WorldmapCheatMenu::menu_action(MenuItem& item)
{
  auto worldmap = worldmap::WorldMap::current();
  auto& tux = worldmap->get_singleton_by_type<worldmap::Tux>();
  assert(worldmap);

  PlayerStatus& status = worldmap->get_savegame().get_player_status();

  switch (item.get_id())
  {
    case MNID_GROW:
      status.bonus = GROWUP_BONUS;
      break;

    case MNID_FIRE:
      status.bonus = FIRE_BONUS;
      status.max_fire_bullets++;
      break;

    case MNID_ICE:
      status.bonus = ICE_BONUS;
      status.max_ice_bullets++;
      break;

    case MNID_AIR:
      status.bonus = AIR_BONUS;
      break;

    case MNID_EARTH:
      status.bonus = EARTH_BONUS;
      break;

    case MNID_SHRINK:
      status.bonus = NO_BONUS;
      break;

    case MNID_GHOST:
      tux.set_ghost_mode(!tux.get_ghost_mode());
      break;

    case MNID_FINISH_LEVEL:
      {
        auto level_tile = worldmap->at_level();
        if (level_tile)
        {
          level_tile->set_solved(true);
          level_tile->set_perfect(false);
        }
      }
      break;

    case MNID_RESET_LEVEL:
      {
        auto level_tile = worldmap->at_level();
        if (level_tile)
        {
          level_tile->set_solved(false);
          level_tile->set_perfect(false);
        }
      }
      break;

    case MNID_FINISH_WORLDMAP:
      worldmap->set_levels_solved(true, false);
      break;

    case MNID_RESET_WORLDMAP:
      worldmap->set_levels_solved(false, false);
      break;

    case MNID_MOVE_TO_LEVEL:
      MenuManager::instance().set_menu(MenuStorage::WORLDMAP_LEVEL_SELECT_MENU);
      return;

    case MNID_MOVE_TO_MAIN:
      worldmap->move_to_spawnpoint("main");
      break;
  }

  MenuManager::instance().clear_menu_stack();
}

WorldmapLevelSelectMenu::WorldmapLevelSelectMenu()
{
  auto worldmap = worldmap::WorldMap::current();
  int id = 0;
  add_label(_("Select level"));
  add_hl();
  for (auto& level : worldmap->get_objects_by_type<worldmap::LevelTile>())
  {
    add_entry(id, level.get_title());
    id++;
  }
  add_hl();
  add_back(_("Back"));
}

void
WorldmapLevelSelectMenu::menu_action(MenuItem& item)
{
  auto worldmap = worldmap::WorldMap::current();
  auto& tux = worldmap->get_singleton_by_type<worldmap::Tux>();
  int id = 0;
  for(const auto& tile : worldmap->get_objects_by_type<worldmap::LevelTile>())
  {
    if(id == item.get_id())
    {
      tux.set_tile_pos(tile.get_pos());
      break;
    }
    id++;
  }
  MenuManager::instance().clear_menu_stack();
}

/* EOF */
