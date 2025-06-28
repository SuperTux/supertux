//  SuperTux
//  Copyright (C) 2025 Martim Ferreira <martim.silva.ferreira@tecnico.ulisboa.pt>
//                2025 Gonçalo Rocha <goncalo.r.f.rocha@tecnico.ulisboa.pt>
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

#include "supertux/menu/multiplayer_online_menu.hpp"

#include "control/controller.hpp"
#include "control/input_manager.hpp"
#include "gui/dialog.hpp"
#include "gui/item_textfield.hpp"
#include "gui/item_toggle.hpp"
#include "gui/menu_item.hpp"
#include "gui/menu_manager.hpp"
#include "object/player.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/ip_manager.hpp"
#include "supertux/menu/menu_storage.hpp"
#include "supertux/online_session_manager.hpp"
#include "supertux/savegame.hpp"
#include "supertux/world.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

MultiplayerOnlineMenu::MultiplayerOnlineMenu() :
  m_ip_name()
{
  OnlineSessionManager::current()->start_thread();
  add_label(_("Multiplayer Online"));
  add_hl();

  add_toggle(-1, _("Host Room"), &g_config->is_hosting)
    .set_help(_("Create a room to host a game online"));

  add_hl();

  add_label(_("Join Room"));

  add_textfield(_("IP"), &m_ip_name)
    .set_help(_("Write the IP of the host whose game you want to join"));

  add_entry(1, _("Join"));

  add_entry(2, _("Enter game"));
  
  if(g_config->is_hosting)
    g_config->is_joining = false;
  else if(g_config->is_joining)
    g_config->is_hosting = false;
    
  add_hl();
  add_back(_("Back"));
}

void
MultiplayerOnlineMenu::menu_action(MenuItem& item)
{
  if (!GameManager::current())
  {
    g_config->left_game = true;
  }
  if (g_config->go_to_level || g_config->left_game)
  {
    std::unique_ptr<World> world = World::from_directory(g_config->world);
    GameManager::current()->start_level(*world, g_config->level);
    g_config->go_to_level = false;
    g_config->is_joining = true;
  }
  if (item.get_id() <= 0)
    return;
  if (!g_config->is_joining)
  {
    try
    {
      if (!m_ip_name.empty() && g_config->is_hosting)
        Dialog::show_message(_("You cannot join a room while hosting one"));
      else if (!m_ip_name.empty())
      {
        if (IPManager::current()->get_ips().size() > 0)
          IPManager::current()->delete_ip(0);
        IPManager::current()->add_ip(m_ip_name);
        InputManager::current()->push_online_user();
        if (GameSession::current() && GameSession::current()->get_savegame().get_player_status().m_num_players < InputManager::current()->get_num_users())
        {
          GameSession::current()->get_savegame().get_player_status().add_player();
          InputManager::current()->m_uses_online_controller[1] = true;
        }
        g_config->is_joining = true;
      }
    }
    catch (const std::exception& err)
    {
      log_warning << "Error updating IP: " << err.what() << std::endl;
      Dialog::show_message(_("An error occurred while updating the IP."));
      return;
    }
  }
}

/* EOF */
