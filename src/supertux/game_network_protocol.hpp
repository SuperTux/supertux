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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_NETWORK_PROTOCOL_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_NETWORK_PROTOCOL_HPP

#include "network/user_protocol.hpp"

#include "supertux/game_network_server_user.hpp"

class GameManager;
class GameSession;

/** Handles online multiplayer events, like starting up a remote GameSession and updating object states. */
class GameNetworkProtocol final : public network::UserProtocol<GameServerUser>
{
public:
  enum Operation
  {
    OP_GAME_JOIN = OP_USER_END,
    OP_GAME_START,
    OP_CONTROLLER_UPDATE,
    OP_GAME_OBJECT_UPDATE,
    OP_END
  };
  enum Channel
  {
    CH_GAME_JOIN_REQUESTS = CH_USER_END,
    CH_CONTROLLER_UPDATES,
    CH_GAME_OBJECT_UPDATES,
    CH_END
  };

public:
  GameNetworkProtocol(GameManager& game_manager, network::Host& host, const std::string& self_nickname);

  std::string get_name() const override { return "game"; }
  size_t get_channel_count() const override { return CH_END; }

private:
  void update() override;

  void on_client_disconnect(network::Peer&, uint32_t code) override;

  bool verify_packet(network::StagedPacket& packet) const override;
  uint8_t get_packet_channel(const network::StagedPacket& packet) const override;

  void on_user_packet_receive(const network::ReceivedPacket& packet, network::ServerUser& user) override;

  void on_request_fail(const network::Request& request, network::Request::FailReason reason) override;

private:
  GameManager& m_game_manager;

  GameSession* m_network_game_session;

private:
  GameNetworkProtocol(const GameNetworkProtocol&) = delete;
  GameNetworkProtocol& operator=(const GameNetworkProtocol&) = delete;
};

#endif

/* EOF */
