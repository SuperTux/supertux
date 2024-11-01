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

#include "network/user_protocol.hpp"

#include "editor/network_server_user.hpp"

class Editor;

/** Handles co-op level editing events in the editor. */
class EditorNetworkProtocol final : public network::UserProtocol<EditorServerUser>
{
public:
  enum Operation
  {
    OP_LEVEL_REQUEST = OP_USER_END,
    OP_LEVEL_REREQUEST,
    OP_LEVEL_RESPONSE,
    OP_LEVEL_RERESPONSE,
    OP_WORLDMAP_RESPONSE,
    OP_WORLDMAP_RERESPONSE,
    OP_SECTOR_CHANGES,
    OP_SECTOR_CREATE,
    OP_SECTOR_DELETE,
    OP_MOUSE_CURSOR_UPDATE,
    OP_END
  };
  enum Channel
  {
    CH_LEVEL_REQUESTS = CH_USER_END,
    CH_SECTOR_CHANGES,
    CH_MOUSE_UPDATES,
    CH_END
  };

public:
  EditorNetworkProtocol(Editor& editor, network::Host& host, const std::string& self_nickname);

  std::string get_name() const override { return "editor"; }
  size_t get_channel_count() const override { return CH_END; }

private:
  void on_server_connect(network::Peer& peer) override;
  void on_server_disconnect(network::Peer& peer, uint32_t code) override;

  void on_client_disconnect(network::Peer&, uint32_t code) override;

  bool verify_packet(network::StagedPacket& packet) const override;
  uint8_t get_packet_channel(const network::StagedPacket& packet) const override;

  void on_packet_abort(network::ReceivedPacket packet) override;
  void on_user_packet_receive(const network::ReceivedPacket& packet, network::ServerUser& user) override;

  network::StagedPacket on_request_receive(const network::ReceivedPacket& packet) override;
  void on_request_fail(const network::Request& request, network::Request::FailReason reason) override;
  void on_request_response(const network::Request& request) override;

private:
  Editor& m_editor;

private:
  EditorNetworkProtocol(const EditorNetworkProtocol&) = delete;
  EditorNetworkProtocol& operator=(const EditorNetworkProtocol&) = delete;
};

#endif

/* EOF */
