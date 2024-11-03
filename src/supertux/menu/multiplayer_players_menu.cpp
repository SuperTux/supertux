//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#include "supertux/menu/multiplayer_players_menu.hpp"

#include <fmt/format.h>

#include "control/game_controller_manager.hpp"
#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/dialog.hpp"
#include "object/player.hpp"
#include "supertux/game_session.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/multiplayer_player_menu.hpp"
#include "util/gettext.hpp"

MultiplayerPlayersMenu::MultiplayerPlayersMenu()
{
  add_label(_("Multiplayer"));
  add_hl();

  for (int i = 0; i < InputManager::current()->get_num_users(); i++)
  {
    add_entry(fmt::format(_("Player {}"), i + 1), [i] {
      MenuManager::instance().push_menu(std::make_unique<MultiplayerPlayerMenu>(i));
    });
  }

  add_hl();

  add_entry(_("Add Player"), [] {
    InputManager::current()->push_user();
    MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayersMenu>());
  });

  if (InputManager::current()->get_num_users() > 1)
  {
    add_entry(_("Remove Last Player"), [] {
      if (GameSession::current())
      {
        const int player_id = InputManager::current()->get_num_users() - 1;
        for (const Player* player : GameSession::current()->get_current_level().get_players())
        {
          if (!player->get_remote_user() && player->get_id() == player_id)
          {
            Dialog::show_confirmation(_("Warning: The player you are trying to\nremove is currently in-game.\n\nDo you wish to remove them anyways?"), []
            {
              InputManager::current()->pop_user();
              MenuManager::instance().pop_menu();
            });
            return;
          }
        }
      }

      InputManager::current()->pop_user();
      MenuManager::instance().pop_menu();
    });
  }

  add_hl();
  add_back(_("Back"));
}

/* EOF */
