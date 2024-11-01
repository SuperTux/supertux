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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_NETWORK_SERVER_USER_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_NETWORK_SERVER_USER_HPP

#include "network/server_user.hpp"

#include <memory>
#include <string>
#include <vector>

#include "control/network_controller.hpp"

/** Represents a remote peer, taking part of a remote game. */
class GameServerUser final : public network::ServerUser
{
public:
  GameServerUser(const std::string& username, Color username_color = Color(1, 1, 1, 1),
                 int num_players = 1);
  GameServerUser(const ReaderMapping& reader);

  void write(Writer& writer) const override;

  int get_num_players() const { return static_cast<int>(player_controllers.size()); }

public:
  // Used by servers to manage remote player input.
  // Used by clients to determine number of players.
  std::vector<std::unique_ptr<NetworkController>> player_controllers;

private:
  GameServerUser(const GameServerUser&) = delete;
  GameServerUser& operator=(const GameServerUser&) = delete;
};

#endif

/* EOF */
