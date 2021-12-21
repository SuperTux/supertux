//  SuperTux
//  Copyright (C) 2015 Matthew <thebatmankiller3@gmail.com>
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

#include "control/game_controller_manager.hpp"
#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/dialog.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"
#include "util/gettext.hpp"

MultiplayerPlayerMenu::MultiplayerPlayerMenu(int player_id)
{
  add_label(_("Player") + " " + std::to_string(player_id + 1));
  add_hl();

  add_toggle(-1, _("Play with the keyboard"), &InputManager::current()->m_uses_keyboard[player_id]);

  add_hl();

  if (InputManager::current()->m_use_game_controller)
  {
    for (auto& pair : InputManager::current()->game_controller_manager->get_controller_mapping())
    {
      auto controller = pair.first;
      std::string prefix = (pair.second == -1) ? "" : (pair.second == player_id) ? "-> " : ("[" + std::to_string(pair.second + 1) + "] ");

      add_entry(prefix + std::string(SDL_GameControllerName(pair.first)), [controller, player_id] {
        InputManager::current()->game_controller_manager->get_controller_mapping()[controller] = player_id;

        // Prevent multiple joysticks to be bound to the same player
        for (auto& pair2 : InputManager::current()->game_controller_manager->get_controller_mapping())
          if (pair2.second == player_id && pair2.first != controller)
            pair2.second = -1;

        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayerMenu>(player_id));
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

        // Prevent multiple joysticks to be bound to the same player
        for (auto& pair2 : InputManager::current()->joystick_manager->get_joystick_mapping())
          if (pair2.second == player_id && pair2.first != joystick)
            pair2.second = -1;

        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayerMenu>(player_id));
      });
    }
  }

  add_hl();

  add_back(_("Back"));
}

/* EOF */
