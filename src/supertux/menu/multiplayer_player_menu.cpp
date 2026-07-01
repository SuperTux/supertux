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

#include <fmt/format.h>
#include <SDL.h>

#include "control/game_controller_manager.hpp"
#include "control/input_manager.hpp"
#include "control/joystick_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/item_toggle.hpp"
#include "gui/item_stringselect.hpp"
#include "object/player.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/savegame.hpp"
#include "supertux/sector.hpp"
#include "worldmap/worldmap.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

int character_type_to_index(CharacterType character_type)
{
  switch(character_type) {
    case CharacterType::PENNY: return 1;
    case CharacterType::LARRY: return 2;
    default: return 0; // TUX
  }
}

CharacterType index_to_character_type(int index)
{
  switch(index) {
    case 1: return CharacterType::PENNY;
    case 2: return CharacterType::LARRY;
    default: return CharacterType::TUX;
  }
}

static PlayerStatus* get_player_status()
{
  if (GameManager::current() && GameManager::current()->m_savegame
      && GameManager::current()->m_savegame->is_title_screen())
    return &GameManager::current()->m_savegame->get_player_status();

  if (worldmap::WorldMap::current())
    return &worldmap::WorldMap::current()->get_savegame().get_player_status();

  if (GameSession::current())
    return &GameSession::current()->get_savegame().get_player_status();

  if (GameManager::current() && GameManager::current()->m_savegame)
    return &GameManager::current()->m_savegame->get_player_status();

  return nullptr;
}

MultiplayerPlayerMenu::MultiplayerPlayerMenu(int player_id) : m_player_id(player_id)
{
  if (GameManager::current())
    GameManager::current()->ensure_savegame_for_profile();

  add_label(fmt::format(_("Player {}"), player_id + 1));
  add_hl();

  add_toggle(-1, _("Play with the keyboard"), &InputManager::current()->m_uses_keyboard[player_id])
    .set_help(_("Don't automatically bind controllers to this player, and spawn it even if it has no controller."));

  m_selected_character = 0;
  PlayerStatus* player_status = get_player_status();
  if (player_status)
    m_selected_character = character_type_to_index(player_status->get_character_id(player_id));

  std::vector<std::string> characters = {
    _("Tux"),
    _("Penny"),
    _("Larry")
  };
  auto& char_select = add_string_select(-1,
                    _("Character"),
                    m_selected_character,
                    characters);
  char_select.set_help(_("Select character for this player (use LEFT/RIGHT arrows)"));
  char_select.set_callback([this](int selected) {
    if (selected >= 0 && selected < 3) {
      PlayerStatus* status = get_player_status();
      if (status) {
        status->set_character_id(m_player_id, index_to_character_type(selected));
        m_selected_character = selected;

        if (GameManager::current() && GameManager::current()->m_savegame
            && GameManager::current()->m_savegame->is_title_screen()
            && !worldmap::WorldMap::current()
            && !GameSession::current()) {
          GameManager::current()->m_savegame->save();
        }
      }
    }
  });

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

        if (!GameSession::current()->on_player_removed(player_id))
        {
          log_warning << "Could not find player with ID " << player_id
                      << " (number " << (player_id + 1) << "in sector"
                      << std::endl;
        }
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

        GameSession::current()->on_player_added(player_id);
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
        InputManager::current()->game_controller_manager->bind_controller(controller, player_id);

        auto err = InputManager::current()->game_controller_manager->rumble(controller);
        switch (err)
        {
          case 1:
            Dialog::show_message(_("This controller does not support rumbling;"
                                   "\nplease check the controllers manually."));
            break;

          case 2:
            Dialog::show_message(_("This SuperTux build does not support "
                                   "rumbling\ncontrollers; please check the "
                                   "controllers manually."));
            break;

          default:
            break;
        }

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
        InputManager::current()->joystick_manager->bind_joystick(joystick, player_id);

        auto err = InputManager::current()->joystick_manager->rumble(joystick);
        switch (err)
        {
          case 1:
            Dialog::show_message(_("This joystick does not support rumbling;"
                                   "\nplease check the joysticks manually."));
            break;

          case 2:
            Dialog::show_message(_("This SuperTux build does not support "
                                   "rumbling\njoysticks; please check the "
                                   "joysticks manually."));
            break;

          default:
            break;
        }

        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayerMenu>(player_id));
      });
    }
  }

  add_hl();

  add_back(_("Back"));
}
