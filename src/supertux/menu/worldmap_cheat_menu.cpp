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
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "util/gettext.hpp"
#include "worldmap/level.hpp"
#include "worldmap/worldmap.hpp"

WorldmapCheatMenu::WorldmapCheatMenu()
{
  add_label(_("Cheats"));
  add_hl();
  add_entry(MNID_GROW, _("Bonus: Grow"));
  add_entry(MNID_FIRE, _("Bonus: Fire"));
  add_entry(MNID_ICE, _("Bonus: Ice"));
  add_entry(MNID_SHRINK, _("Bonus: None"));
  add_hl();
  add_entry(MNID_FINISH_LEVEL, _("Finish Level"));
  add_entry(MNID_RESET_LEVEL, _("Reset Level"));
  add_hl();
  add_entry(MNID_FINISH_WORLDMAP, _("Finish WorldMap"));
  add_entry(MNID_RESET_WORLDMAP, _("Reset WorldMap"));
  add_hl();
  add_back(_("Back"));
}

void
WorldmapCheatMenu::menu_action(MenuItem* item)
{
  worldmap::WorldMap* worldmap = worldmap::WorldMap::current();
  if (!worldmap)
  {
    log_warning << "couldn't access WorldMap::current()" << std::endl;
  }
  else
  {
    PlayerStatus* status = worldmap->get_savegame().get_player_status();

    switch(item->id)
    {
      case MNID_GROW:
        status->bonus = GROWUP_BONUS;
        break;

      case MNID_FIRE:
        status->bonus = FIRE_BONUS;
        break;

      case MNID_ICE:
        status->bonus = ICE_BONUS;
        break;

      case MNID_SHRINK:
        status->bonus = NO_BONUS;
        break;

      case MNID_FINISH_LEVEL:
        {
          worldmap::LevelTile* level_tile = worldmap->at_level();
          if (level_tile)
          {
            level_tile->set_solved(true);
            level_tile->set_perfect(false);
          }
        }
        break;

      case MNID_RESET_LEVEL:
        {
          worldmap::LevelTile* level_tile = worldmap->at_level();
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
    }
  }

  MenuManager::instance().clear_menu_stack();
}

/* EOF */
