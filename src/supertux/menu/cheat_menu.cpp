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
#include "supertux/menu/cheat_apply_menu.hpp"
#include "supertux/sector.hpp"

CheatMenu::CheatMenu()
{


  add_label(_("Cheats"));
  add_hl();
  add_entry(MNID_GROW, _("Bonus: Grow"));
  add_entry(MNID_FIRE, _("Bonus: Fire"));
  add_entry(MNID_ICE, _("Bonus: Ice"));
  add_entry(MNID_AIR, _("Bonus: Air"));
  add_entry(MNID_EARTH, _("Bonus: Earth"));
  add_entry(MNID_STAR, _("Bonus: Star"));
  add_entry(MNID_SHRINK, _("Shrink Tux"));
  add_entry(MNID_KILL, _("Kill Tux"));
  add_entry(MNID_FINISH, _("Finish Level"));
  if (GameSession::current())
  {
    std::vector<Player*> local_players;
    for (Player& player : GameSession::current()->get_current_sector().get_objects_by_type<Player>())
    {
      if (!player.get_remote_user())
        local_players.push_back(&player);
    }

    if (local_players.size() == 1)
    {
      add_entry(MNID_GHOST, local_players[0]->get_ghost_mode() ?
                _("Leave Ghost Mode") : _("Activate Ghost Mode"));
    }
    else
    {
      // In multiplayer, different players may have different ghost states.
      add_entry(MNID_GHOST, _("Activate Ghost Mode"));
      add_entry(MNID_UNGHOST, _("Leave Ghost Mode"));
    }
  }

  if (GameSession::current())
    add_toggle(-1, _("Prevent Death"), &GameSession::current()->m_prevent_death);

  add_hl();
  add_back(_("Back"));
}

void
CheatMenu::menu_action(MenuItem& item)
{
  if (!GameSession::current()) return;

  // TODO: Cheat menu support for remote players
  Player* single_player = nullptr;
  for (Player* player : GameSession::current()->get_current_level().get_players())
  {
    if (!player->get_remote_user())
    {
      single_player = player;
      break;
    }
  }

  switch (item.get_id())
  {
    case MNID_GROW:
      if (single_player)
      {
        single_player->set_bonus(GROWUP_BONUS);
        MenuManager::instance().clear_menu_stack();
      }
      else
      {
        MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player){
          player.set_bonus(GROWUP_BONUS);
        }));
      }
      break;

    case MNID_FIRE:
      MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player, int count){
        player.set_bonus(FIRE_BONUS);
        player.get_status().max_fire_bullets = count;
      }));
      break;

    case MNID_ICE:
      MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player, int count){
        player.set_bonus(ICE_BONUS);
        player.get_status().max_ice_bullets = count;
      }));
      break;

    case MNID_AIR:
      MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player, int count){
        player.set_bonus(AIR_BONUS);
        player.get_status().max_air_time = count;
      }));
      break;

    case MNID_EARTH:
      MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player, int count){
        player.set_bonus(EARTH_BONUS);
        player.get_status().max_earth_time = count;
      }));
      break;

    case MNID_STAR:
      if (single_player)
      {
        single_player->make_invincible();
        MenuManager::instance().clear_menu_stack();
      }
      else
      {
        MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player){
          player.make_invincible();
        }));
      }
      break;

    case MNID_SHRINK:
      if (single_player)
      {
        single_player->kill(false);
        MenuManager::instance().clear_menu_stack();
      }
      else
      {
        MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player){
          player.kill(false);
        }));
      }
      break;

    case MNID_KILL:
      if (single_player)
      {
        single_player->kill(true);
        MenuManager::instance().clear_menu_stack();
      }
      else
      {
        MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([](Player& player){
          player.kill(true);
        }));
      }
      break;

    case MNID_FINISH:
      GameSession::current()->finish(true);
      MenuManager::instance().clear_menu_stack();
      break;

    case MNID_GHOST:
    case MNID_UNGHOST:
      if (single_player)
      {
        single_player->set_ghost_mode(!single_player->get_ghost_mode());
        MenuManager::instance().clear_menu_stack();
      }
      else
      {
        MenuManager::instance().push_menu(std::make_unique<CheatApplyMenu>([&item](Player& player){
          player.set_ghost_mode(item.get_id() == MNID_GHOST);
        }));
      }
      break;

    default:
      break;
  }
}

/* EOF */
