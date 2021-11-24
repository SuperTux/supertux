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

#include "supertux/menu/cheat_menu.hpp"

#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "object/player.hpp"
#include "scripting/functions.hpp"
#include "supertux/game_session.hpp"
#include "supertux/sector.hpp"

// FIXME: Everything here affects only the first player

CheatMenu::CheatMenu()
{
  auto& player = *(Sector::get().get_players()[0]);

  add_label(_("Cheats"));
  add_hl();
  add_entry(MNID_GROW, _("Bonus: Grow"));
  add_entry(MNID_FIRE, _("Bonus: Fire x 64"));
  add_entry(MNID_ICE, _("Bonus: Ice x 64"));
  add_entry(MNID_AIR, _("Bonus: Air x 64"));
  add_entry(MNID_EARTH, _("Bonus: Earth x 64"));
  add_entry(MNID_STAR, _("Bonus: Star"));
  add_entry(MNID_SHRINK, _("Shrink Tux"));
  add_entry(MNID_KILL, _("Kill Tux"));
  add_entry(MNID_FINISH, _("Finish Level"));
  add_entry(MNID_GHOST, player.get_ghost_mode() ?
            _("Leave Ghost Mode") : _("Activate Ghost Mode"));
  add_hl();
  add_back(_("Back"));
}

void
CheatMenu::menu_action(MenuItem& item)
{
  if (!Sector::current()) return;

  auto& player = *(Sector::get().get_players()[0]);

  switch (item.get_id())
  {
    case MNID_GROW:
      player.set_bonus(GROWUP_BONUS);
      break;

    case MNID_FIRE:
      player.set_bonus(FIRE_BONUS);
      player.get_status().max_fire_bullets = 64;
      break;

    case MNID_ICE:
      player.set_bonus(ICE_BONUS);
      player.get_status().max_ice_bullets = 64;
      break;

    case MNID_AIR:
      player.set_bonus(AIR_BONUS);
      player.get_status().max_air_time = 64;
      break;

    case MNID_EARTH:
      player.set_bonus(EARTH_BONUS);
      player.get_status().max_earth_time = 64;
      break;

    case MNID_STAR:
      player.make_invincible();
      break;

    case MNID_SHRINK:
      player.kill(false);
      break;

    case MNID_KILL:
      player.kill(true);
      break;

    case MNID_FINISH:
      if (GameSession::current())
      {
        GameSession::current()->finish(true);
      }
      break;

    case MNID_GHOST:
      if (GameSession::current())
      {
        if (player.get_ghost_mode())
        {
          scripting::mortal();
        }
        else
        {
          scripting::ghost();
        }
      }
      break;

    default:
      break;
  }

  MenuManager::instance().clear_menu_stack();
}

/* EOF */
