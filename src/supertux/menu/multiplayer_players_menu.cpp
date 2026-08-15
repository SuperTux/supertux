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
#include "gui/menu_manager.hpp"
#include "object/character_registry.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/menu/character_selector_menu.hpp"
#include "supertux/menu/multiplayer_player_menu.hpp"
#include "supertux/savegame.hpp"
#include "supertux/sector.hpp"
#include "worldmap/worldmap.hpp"
#include "object/player.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

MultiplayerPlayersMenu::MultiplayerPlayersMenu()
{
  if (GameManager::current())
    GameManager::current()->ensure_savegame_for_profile();
  refresh();
}

void
MultiplayerPlayersMenu::refresh()
{
  clear();

  add_label(_("Multiplayer"));
  add_hl();

  for (int i = 0; i < InputManager::current()->get_num_users(); i++)
  {
    add_entry(fmt::format(_("Player {}"), i + 1), [i] {
      MenuManager::instance().push_menu(std::make_unique<MultiplayerPlayerMenu>(i));
    });
  }

  add_hl();

  if (InputManager::current()->can_add_user())
  {
    auto on_character_selected = [](int character_id) {
      PlayerStatus* status = nullptr;

      if (GameManager::current() && GameManager::current()->m_savegame
          && GameManager::current()->m_savegame->is_title_screen())
      {
        status = &GameManager::current()->m_savegame->get_player_status();
      }
      else if (worldmap::WorldMap::current())
      {
        status = &worldmap::WorldMap::current()->get_savegame().get_player_status();
      }
      else if (GameSession::current())
      {
        status = &GameSession::current()->get_savegame().get_player_status();
      }
      else if (GameManager::current() && GameManager::current()->m_savegame)
      {
        status = &GameManager::current()->m_savegame->get_player_status();
      }

      if (!status)
        return;

      const char* char_ids[] = {"tux", "penny", "larry"};
      int player_index = InputManager::current()->get_num_users() - 1;

      if (status->m_num_players <= player_index)
        status->add_player();

      if (character_id >= 0 && character_id < 3)
        status->set_character_id(player_index, CharacterRegistry::string_to_character(char_ids[character_id]));

      MenuManager::instance().pop_menu();
      if (MenuManager::instance().current_menu())
        MenuManager::instance().current_menu()->refresh();
    };

    add_entry(_("Add Player (Keyboard)"), [this, on_character_selected] {
      InputManager::current()->push_user();
      InputManager::current()->m_uses_keyboard[InputManager::current()->get_num_users() - 1] = true;
      MenuManager::instance().push_menu(std::make_unique<CharacterSelectorMenu>(
        on_character_selected,
        [this]() { InputManager::current()->pop_user(); }
      ));
    });

    add_entry(_("Add Player (Controller)"), [this, on_character_selected] {
      InputManager::current()->push_user();
      MenuManager::instance().push_menu(std::make_unique<CharacterSelectorMenu>(
        on_character_selected,
        [this]() { InputManager::current()->pop_user(); }
      ));
    });
  }

  if (InputManager::current()->get_num_users() > 1)
  {
    add_entry(_("Remove Last Player"), [] {
      if (Sector::current() && Sector::current()->get_object_count<Player>() >= InputManager::current()->get_num_users())
      {
        Dialog::show_confirmation(_("Warning: The player you are trying to\nremove is currently in-game.\n\nDo you wish to remove them anyways?"), [] {

          // Remove the player before the controller, else it'll behave funny
          int num = InputManager::current()->get_num_users();
          // FIXME: Probably not a good idea to get a player by name
          Player* player = Sector::current()->get_object_by_name<Player>("Tux" + std::to_string(num));

          if (player)
          {
            player->remove_me();
          }
          else
          {
            log_warning << "Cannot disconnect player #" << num << " in a sector with " << Sector::current()->get_object_count<Player>() << std::endl;
          }

          InputManager::current()->pop_user();
          MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayersMenu>());
        });
      }
      else
      {
        InputManager::current()->pop_user();
        MenuManager::instance().set_menu(std::make_unique<MultiplayerPlayersMenu>());
      }
    });
  }

  add_hl();
  add_back(_("Back"));
}
