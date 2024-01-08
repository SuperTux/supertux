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

#ifndef HEADER_SUPERTUX_NETWORK_PROTOCOL_HPP
#define HEADER_SUPERTUX_NETWORK_PROTOCOL_HPP

#include <enet/enet.h>

#include "network/connection_result.hpp"
#include "network/packet.hpp"
#include "network/peer.hpp"
#include "network/request.hpp"

namespace network {

/** Represents a protocol, which handles events, occuring on the host
    its binded with. */
class Protocol
{
public:
  Protocol() {}
  virtual ~Protocol() {}

  /** Used as a unique identifier for the protocol
      to confirm a remote peer, receiving a packet,
      is running the same protocol as the one sending out the packet. */
  virtual std::string get_name() const = 0;

  /** The protocol is updated every time the host it's binded to is.
      Called after all events have been processed. */
  virtual void update() {}

  virtual void on_server_connect(Peer& peer) {}
  virtual void on_server_disconnect(Peer& peer) {}

  virtual void on_client_connect(const ConnectionResult& result) {}
  virtual void on_client_disconnect(Peer& peer) {}

  /** Return value indicates whether the packet is valid and can be sent over. */
  virtual bool verify_packet(const StagedPacket& packet) = 0;

  /** The provided RecievedPacket represents the packet in the same way
      the remote peer would have recieved it. `packet.peer` is not set. */
  virtual void on_packet_send(const RecievedPacket& packet) {}
  virtual void on_packet_abort(const RecievedPacket& packet) {}
  virtual void on_packet_recieve(const RecievedPacket& packet) {}

  /** On request, a staged packet to be sent back must be provided. */
  virtual StagedPacket on_request_recieve(const RecievedPacket& packet) { return {}; }
  virtual void on_request_fail(const Request& request, Request::FailReason reason) {}
  virtual void on_request_response(const Request& request) {}

private:
  Protocol(const Protocol&) = delete;
  Protocol& operator=(const Protocol&) = delete;
};

} // namespace network

#endif

/* EOF */
