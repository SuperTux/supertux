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

#ifndef HEADER_SUPERTUX_EDITOR_NETWORK_PROTOCOL_HPP
#define HEADER_SUPERTUX_EDITOR_NETWORK_PROTOCOL_HPP

#include "network/protocol.hpp"

#include <unordered_map>

class Editor;
class Timer;

namespace network {
class Host;
} // namespace network

/** Handles co-op level editing events in the editor. */
class EditorNetworkProtocol final : public network::Protocol
{
public:
  static bool verify_nickname(const std::string& nickname);

public:
  enum DisconnectionReason
  {
    DISCONNECTED_REGISTER_TIMED_OUT // The time for the client to send a registration packet in has expired.
      = network::DISCONNECTION_REASONS_END,
    DISCONNECTED_NICKNAME_INVALID, // The provided nickname is invalid.
    DISCONNECTED_NICKNAME_TAKEN // The provided nickname has already been taken by another client.
  };
  enum Operation
  {
    OP_USER_REGISTER,
    OP_USERS_REQUEST,
    OP_USERS_RESPONSE,
    OP_USER_SERVER_CONNECT,
    OP_USER_SERVER_DISCONNECT,
    OP_LEVEL_REQUEST,
    OP_LEVEL_REREQUEST,
    OP_LEVEL_RESPONSE,
    OP_LEVEL_RERESPONSE,
    OP_SECTOR_CHANGES,
    OP_SECTOR_CREATE,
    OP_SECTOR_DELETE,
    OP_MOUSE_CURSOR_UPDATE,
    OP_END
  };
  enum Channel
  {
    CH_USER_SERVER_CONNECTIONS,
    CH_LEVEL_REQUESTS,
    CH_SECTOR_CHANGES,
    CH_MOUSE_UPDATES,
    CH_END
  };

public:
  EditorNetworkProtocol(Editor& editor, network::Host& host);

  std::string get_name() const override { return "editor"; }
  size_t get_channel_count() const override { return CH_END; }

  void update() override;

  void on_server_connect(network::Peer& peer) override;
  void on_server_disconnect(network::Peer& peer, uint32_t) override;

  void on_client_disconnect(network::Peer&, uint32_t code) override;

  bool allow_packet_send(network::Peer& peer) const override;
  bool verify_packet(network::StagedPacket& packet) const override;
  uint8_t get_packet_channel(const network::StagedPacket& packet) const override;

  void on_packet_abort(network::ReceivedPacket packet) override;
  void on_packet_receive(network::ReceivedPacket packet) override;

  network::StagedPacket on_request_receive(const network::ReceivedPacket& packet) override;
  void on_request_fail(const network::Request& request, network::Request::FailReason reason) override;
  void on_request_response(const network::Request& request) override;

private:
  Editor& m_editor;
  network::Host& m_host;

  /** Pending users without a recieved registration data packet. */
  std::unordered_map<ENetPeer*, std::unique_ptr<Timer>> m_pending_users;

private:
  EditorNetworkProtocol(const EditorNetworkProtocol&) = delete;
  EditorNetworkProtocol& operator=(const EditorNetworkProtocol&) = delete;
};

#endif

/* EOF */
