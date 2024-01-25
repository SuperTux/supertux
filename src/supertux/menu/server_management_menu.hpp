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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_SERVER_MANAGEMENT_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_SERVER_MANAGEMENT_MENU_HPP

#include "gui/menu.hpp"

namespace network {
class RemoteUser;
class Server;
} // namespace network

class ServerManagementMenu final : public Menu
{
private:
  enum MenuIDs
  {
    MNID_ACTION_MENU = -1,
    MNID_KICK = -2,
    MNID_BAN = -3,
    MNID_BANLIST = -4,
    MNID_WHITELIST = -5,
    MNID_RESTRICT_MODE = -6
  };

public:
  ServerManagementMenu(network::Server& server);

  void refresh() override;
  void menu_action(MenuItem& item) override;

private:
  void rebuild_menu();

private:
  network::Server& m_server;
  std::vector<network::RemoteUser> m_users;

  int m_selected_user;

private:
  ServerManagementMenu(const ServerManagementMenu&) = delete;
  ServerManagementMenu& operator=(const ServerManagementMenu&) = delete;
};

#endif

/* EOF */
