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

#include "supertux/menu/multiplayer_player_menu.hpp"

#include "boost/format.hpp"
#include "SDL.h"

#include "control/game_controller_manager.hpp"
#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/dialog.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/savegame.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

MultiplayerPlayerMenu::MultiplayerPlayerMenu(int player_id)
{
  add_label((boost::format(_("Player %d")) % (player_id + 1)).str());
  add_hl();

  add_toggle(-1, _("Play with the keyboard"), &InputManager::current()->m_uses_keyboard[player_id]);

  if (player_id != 0 && GameSession::current()
      && !GameSession::current()->get_savegame().is_title_screen())
  {
    bool player_is_in_sector = false;

    for (const auto* player : GameSession::current()->get_current_sector().get_players())
    {
      if (player->get_id() == player_id)
      {
        player_is_in_sector = true;
        break;
      }
    }

    if (player_is_in_sector)
    {
      add_entry(_("Remove Player"), [player_id] {
        // Re-check everything that concerns the sector, it might have changed
        // (e. g. when unplugging a controller with auto-management enabled)
        if (!GameSession::current() || GameSession::current()->get_savegame().is_title_screen())
        {
          log_warning << "Attempt to force player to despawn while not in session"
                      << std::endl;
          return;
        }

        for (auto* player : GameSession::current()->get_current_sector().get_players())
        {
          if (player->get_id() == player_id)
          {
            player->remove_me();
            return;
          }
        }

        log_warning << "Could not find player with ID " << player_id
                    << " (number " << (player_id + 1) << "in sector"
                    << std::endl;
      });

      add_entry(_("Respawn Player"), [player_id] {
        // Re-check everything that concerns the sector, it might have changed
        // (e. g. when unplugging a controller with auto-management enabled)
        if (!GameSession::current() || GameSession::current()->get_savegame().is_title_screen())
        {
          log_warning << "Attempt to force player to respawn while not in session"
                      << std::endl;
          return;
        }

        for (auto* player : GameSession::current()->get_current_sector().get_players())
        {
          if (player->get_id() == player_id)
          {
            player->multiplayer_prepare_spawn();
            return;
          }
        }

        log_warning << "Could not find player with ID " << player_id
                    << " (number " << (player_id + 1) << "in sector"
                    << std::endl;
      });
    }
    else
    {
      add_entry(_("Spawn Player"), [player_id] {
        // Re-check everything that concerns the sector, it might have changed
        // (e. g. when unplugging a controller with auto-management enabled)
        if (!GameSession::current() || GameSession::current()->get_savegame().is_title_screen())
        {
          log_warning << "Attempt to force player to spawn while not in session"
                      << std::endl;
          return;
        }

        auto& sector = GameSession::current()->get_current_sector();
        auto& player_status = GameSession::current()->get_savegame().get_player_status();

        // TODO: This is probably needed because adding/removing users manually
        // or automatically by plugging in controllers might not always fix the
        // player_status object; check if that statement is correct
        if (player_status.m_num_players <= player_id)
          player_status.add_player();

        // ID = 0 is impossible, so no need to write `(id == 0) ? "" : ...`
        auto& player = sector.add<Player>(player_status, "Tux" + std::to_string(player_id + 1), player_id);

        player.multiplayer_prepare_spawn();
      });
    }
  }

  add_hl();
  add_label(_("Controllers"));

  if (InputManager::current()->m_use_game_controller)
  {
    for (auto& pair : InputManager::current()->game_controller_manager->get_controller_mapping())
    {
      auto controller = pair.first;
      std::string prefix = (pair.second == -1) ? "" : (pair.second == player_id) ? "-> " : ("[" + std::to_string(pair.second + 1) + "] ");

      add_entry(prefix + std::string(SDL_GameControllerName(pair.first)), [controller, player_id] {
        InputManager::current()->game_controller_manager->get_controller_mapping()[controller] = player_id;

        if (!g_config->multiplayer_multibind)
          for (auto& pair2 : InputManager::current()->game_controller_manager->get_controller_mapping())
            if (pair2.second == player_id && pair2.first != controller)
              pair2.second = -1;

        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayerMenu>(player_id));

#if SDL_VERSION_ATLEAST(2, 0, 9)
        if (g_config->multiplayer_buzz_controllers)
        {
#if SDL_VERSION_ATLEAST(2, 0, 18)
          if (SDL_GameControllerHasRumble(controller))
          {
#endif
            // TODO: Rumble intensity setting (like volume)
            SDL_GameControllerRumble(controller, 0xFFFF, 0xFFFF, 300);
#if SDL_VERSION_ATLEAST(2, 0, 18)
          }
          else
          {
            Dialog::show_message(_("This controller does not support rumbling;\n"
                                    "please check the controllers manually."));
          }
#endif
        }
#else
        Dialog::show_message(_("This SuperTux build does not support rumbling\n"
                                "controllers; please check the controllers manually.\n"
                                "\nYou may disable this message in the Options menu."));
#endif
      });
    }
  }
  else
  {
    for (auto& pair : InputManager::current()->joystick_manager->get_joystick_mapping())
    {
      auto joystick = pair.first;
      std::string prefix = (pair.second == -1) ? "" : (pair.second == player_id) ? "-> " : ("[" + std::to_string(pair.second + 1) + "] ");

      add_entry(prefix + std::string(SDL_JoystickName(pair.first)), [joystick, player_id] {
        InputManager::current()->joystick_manager->get_joystick_mapping()[joystick] = player_id;

        if (!g_config->multiplayer_multibind)
          for (auto& pair2 : InputManager::current()->joystick_manager->get_joystick_mapping())
            if (pair2.second == player_id && pair2.first != joystick)
              pair2.second = -1;

        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayerMenu>(player_id));

#if SDL_VERSION_ATLEAST(2, 0, 9)
        if (g_config->multiplayer_buzz_controllers)
        {
#if SDL_VERSION_ATLEAST(2, 0, 18)
          if (SDL_JoystickHasRumbleTriggers(joystick))
          {
#endif
            // TODO: Rumble intensity setting (like volume)
            SDL_JoystickRumble(joystick, 0xFFFF, 0xFFFF, 300);
#if SDL_VERSION_ATLEAST(2, 0, 18)
          }
          else
          {
            Dialog::show_message(_("This joystick does not support rumbling;\n"
                                    "please check the joysticks manually."));
          }
#endif
        }
#else
        Dialog::show_message(_("This SuperTux build does not support rumbling\n"
                                "joysticks; please check the joysticks manually.\n"
                                "\nYou may disable this message in the Options menu."));
#endif
      });
    }
  }

  add_hl();

  add_back(_("Back"));
}

/* EOF */
