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
#include "supertux/game_session.hpp"
#include "supertux/player_status.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"

CheatMenu::CheatMenu()
{
  add_label(_("Cheats"));
  add_hl();
  add_entry(MNID_GROW, _("Bonus: Grow"));
  add_entry(MNID_FIRE, _("Bonus: Fire"));
  add_entry(MNID_ICE, _("Bonus: Ice"));
  add_entry(MNID_STAR, _("Bonus: Star"));
  add_entry(MNID_SHRINK, _("Shrink Tux"));
  add_entry(MNID_KILL, _("Kill Tux"));
  add_entry(MNID_FINISH, _("Finish Level"));
  add_hl();
  add_back(_("Back"));
}

void
CheatMenu::menu_action(MenuItem* item)
{
  if (Sector::current())
  {
    std::vector<Player*> players = Sector::current()->get_players();
    Player* player = players.empty() ? nullptr : players[0];

    switch(item->id)
    {
      case MNID_GROW:
        if (player)
        {
          player->set_bonus(GROWUP_BONUS);
        }
        break;

      case MNID_FIRE:
        if (player)
        {
          player->set_bonus(FIRE_BONUS);
        }
        break;

      case MNID_ICE:
        if (player)
        {
          player->set_bonus(ICE_BONUS);
        }
        break;

      case MNID_STAR:
        if (player)
        {
          player->make_invincible();
        }
        break;

      case MNID_SHRINK:
        if (player)
        {
          player->kill(false);
        }
        break;

      case MNID_KILL:
        if (player)
        {
          player->kill(true);
        }
        break;

      case MNID_FINISH:
        if (GameSession::current())
        {
          GameSession::current()->finish(true);
        }
        break;

      default:
        break;
    }
  }

  MenuManager::instance().clear_menu_stack();
}

/* EOF */
