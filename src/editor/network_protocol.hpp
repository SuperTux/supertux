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

class Editor;

/** Handles co-op level editing events in the editor. */
class EditorNetworkProtocol final : public network::Protocol
{
public:
  enum Operation
  {
    OP_LEVEL_REQUEST,
    OP_LEVEL_REREQUEST,
    OP_LEVEL_RESPONSE,
    OP_LEVEL_RERESPONSE,
    //OBJECT_ACTION_EVENT,
    OP_END
  };

public:
  EditorNetworkProtocol(Editor& editor);

  std::string get_name() const override { return "editor"; }

  void on_server_connect(network::Peer&) override;
  void on_server_disconnect(network::Peer&) override;

  void on_client_disconnect(network::Peer&) override;

  bool verify_packet(const network::StagedPacket& packet) override;

  network::StagedPacket on_request_recieve(const network::RecievedPacket& packet) override;
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
