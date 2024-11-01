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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_NETWORK_CONNECT_MENU_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_NETWORK_CONNECT_MENU_HPP

#include "gui/menu.hpp"

#include "video/color.hpp"

class NetworkConnectMenu final : public Menu
{
public:
  NetworkConnectMenu(bool game, bool connect);

  void menu_action(MenuItem& item) override;

private:
  /** True - regarding a remote game.
      False - regarding remote level editing. */
  const bool m_game;
  /** True - connecting to server.
      False - hosting level. */
  const bool m_connect;

  std::string m_host_address;
  int m_port;

  std::string m_username;
  Color m_username_color;

private:
  NetworkConnectMenu(const NetworkConnectMenu&) = delete;
  NetworkConnectMenu& operator=(const NetworkConnectMenu&) = delete;
};

#endif

/* EOF */
