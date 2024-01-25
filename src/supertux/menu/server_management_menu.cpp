//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#include "supertux/menu/server_management_menu.hpp"

#include <fmt/format.h>

#include "gui/dialog.hpp"
#include "gui/item_horizontalmenu.hpp"
#include "gui/menu_manager.hpp"
#include "gui/menu_string_array.hpp"
#include "network/server.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "util/gettext.hpp"

ServerManagementMenu::ServerManagementMenu(network::Server& server) :
  m_server(server),
  m_users(),
  m_selected_user(-1)
{
  refresh();
}

void
ServerManagementMenu::refresh()
{
  m_users = m_server.get_users();
  m_selected_user = -1;

  rebuild_menu();
}

void
ServerManagementMenu::rebuild_menu()
{
  clear();

  add_label(_("Manage Server"));
  add_hl();

  if (m_users.empty())
  {
    add_inactive(_("No connected peers."));
  }
  else
  {
    for (int i = 0; i < static_cast<int>(m_users.size()); i++)
      add_entry(i, m_selected_user == i ? "[" + m_users[i].display_text + "]" :
                     m_users[i].display_text);
  }
  add_hl();

  ItemHorizontalMenu& action_menu = add_horizontalmenu(MNID_ACTION_MENU, 120.f, 100.f);
  action_menu.add_item(_("Kick"), _("Kick the selected user from this server."),
                       "images/engine/editor/ambient_light.png", MNID_KICK, m_selected_user < 0);
  if (g_config->network_restrict_mode == Config::NetworkRestrictMode::BLACKLIST)
  {
    action_menu.add_item(_("Ban"), _("Ban the selected user from this server."),
                         "images/engine/editor/ambientsound.png", MNID_BAN, m_selected_user < 0);
    action_menu.add_item(_("Blacklist"), _("Edit this server's blacklist (ban list)."),
                         "images/engine/editor/climbable.png", MNID_BANLIST);
  }
  else // Whitelist
  {
    action_menu.add_item(_("Whitelist"), _("Edit this server's whitelist."),
                         "images/engine/editor/decal.png", MNID_WHITELIST);
  }

  add_hl();
  add_string_select(MNID_RESTRICT_MODE, _("Restriction Mode"),
                    reinterpret_cast<int*>(&g_config->network_restrict_mode),
                    { _("Blacklist"), _("Whitelist") });

  add_hl();
  add_back(_("Back"));
}

void
ServerManagementMenu::menu_action(MenuItem& item)
{
  if (item.get_id() >= 0)
  {
    if (m_selected_user == item.get_id()) return;

    m_selected_user = item.get_id();
    rebuild_menu();
    set_active_item(m_selected_user);
  }
  else if (item.get_id() == MNID_ACTION_MENU)
  {
    const auto& horizontal_item = static_cast<ItemHorizontalMenu&>(item).get_selected_item();
    if (horizontal_item.disabled) return;

    switch (horizontal_item.id)
    {
      case MNID_KICK:
        Dialog::show_confirmation(fmt::format(fmt::runtime(_("Are you sure you want to kick \"{}\"?")),
                                              m_users[m_selected_user].display_text),
          [this]() {
            m_server.kick(&m_users[m_selected_user].peer.enet);
          });
        break;

      case MNID_BAN:
        Dialog::show_confirmation(fmt::format(fmt::runtime(_("This will ban all users on the host address \"{}\",\nincluding \"{}\".\n \nAre you sure?")),
                                              m_users[m_selected_user].peer.address.host,
                                              m_users[m_selected_user].display_text),
          [this]() {
            m_server.ban(&m_users[m_selected_user].peer.enet);
          });
        break;

      case MNID_BANLIST:
        MenuManager::instance().push_menu(std::make_unique<StringArrayMenu>(g_config->network_blacklist, _("Edit Blacklist")));
        break;

      case MNID_WHITELIST:
        MenuManager::instance().push_menu(std::make_unique<StringArrayMenu>(g_config->network_whitelist, _("Edit Whitelist")));
        break;

      default:
        break;
    }
  }
  else if (item.get_id() == MNID_RESTRICT_MODE)
  {
    rebuild_menu();
    set_active_item(MNID_RESTRICT_MODE);
  }
}

/* EOF */
