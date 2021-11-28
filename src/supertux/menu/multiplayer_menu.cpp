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

#include "supertux/menu/multiplayer_menu.hpp"

#include "control/game_controller_manager.hpp"
#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/dialog.hpp"
#include "supertux/sector.hpp"
#include "object/player.hpp"
#include "util/gettext.hpp"

MultiplayerMenu::MultiplayerSelectMenu::MultiplayerSelectMenu(int player_id)
{
  if (InputManager::current()->m_use_game_controller)
  {
    for (auto& pair : InputManager::current()->game_controller_manager->get_controller_mapping())
    {
      auto controller = pair.first;
      add_entry(std::string(SDL_GameControllerName(pair.first)), [controller, player_id] {
        InputManager::current()->game_controller_manager->get_controller_mapping()[controller] = player_id;

        // Prevent multiple joysticks to be bound to the same player
        for (auto& pair2 : InputManager::current()->game_controller_manager->get_controller_mapping())
          if (pair2.second == player_id && pair2.first != controller)
            pair2.second = -1;

        MenuManager::instance().pop_menu();
      });
    }
  }
  else
  {
    for (auto& pair : InputManager::current()->joystick_manager->get_joystick_mapping())
    {
      auto joystick = pair.first;

      add_entry(std::string(SDL_JoystickName(pair.first)), [joystick, player_id] {
        InputManager::current()->joystick_manager->get_joystick_mapping()[joystick] = player_id;

        // Prevent multiple joysticks to be bound to the same player
        for (auto& pair2 : InputManager::current()->joystick_manager->get_joystick_mapping())
          if (pair2.second == player_id && pair2.first != joystick)
            pair2.second = -1;

        MenuManager::instance().pop_menu();
      });
    }
  }

  add_back(_("Back"));
}

MultiplayerMenu::MultiplayerMenu()
{
  add_label(_("Multiplayer"));
  add_hl();

  for (int i = 0; i < InputManager::current()->get_num_users(); i++)
  {
    add_entry(_("Player") + " " + std::to_string(i + 1), [i] {
      MenuManager::instance().push_menu(std::make_unique<MultiplayerSelectMenu>(i));
    });
  }

  add_hl();

  add_entry(_("Add Player"), [] {
    InputManager::current()->push_user();
    MenuManager::instance().set_menu(std::make_unique<MultiplayerMenu>());
  });

  if (InputManager::current()->get_num_users() > 1)
  {
    add_entry(_("Remove Last Player"), [] {
      if (Sector::current() && Sector::current()->get_object_count<Player>() >= InputManager::current()->get_num_users())
      {
        Dialog::show_confirmation(_("Warning: The player you are trying to\nremove is currently in-game.\n\nDo you wish to remove them anyways?"), [] {

          // Remove the player before the controller, else it'll behave funny
          auto num = InputManager::current()->get_num_users();
          auto player = Sector::current()->get_object_by_name<Player>("Tux" + std::to_string(num));

          if (player)
          {
            player->remove_me();
          }
          else
          {
            log_warning << "Cannot disconnect player #" << num << " in a sector with " << Sector::current()->get_object_count<Player>() << std::endl;
          }

          InputManager::current()->pop_user();
          MenuManager::instance().set_menu(std::make_unique<MultiplayerMenu>());
        });
      }
      else
      {
        InputManager::current()->pop_user();
        MenuManager::instance().set_menu(std::make_unique<MultiplayerMenu>());
      }
    });
  }

  add_hl();
  add_back(_("Back"));
}

/* EOF */
