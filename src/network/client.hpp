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

#ifndef HEADER_SUPERTUX_NETWORK_CLIENT_HPP
#define HEADER_SUPERTUX_NETWORK_CLIENT_HPP

#include "network/host.hpp"

namespace network {

/** A client, which can connect to servers. */
class Client final : public Host
{
public:
  enum class ConnectionStatus
  {
    SUCCESS, // Connected to peer successfully. Available via the "peer" pointer.
    FAILED_NO_PEERS, // Failed (no peers are currently available).
    FAILED_TIMED_OUT, // Failed (connection timed out).
    FAILED_DISCONNECT // Failed (a disconnect event was recieved instead).
  };
  struct ConnectionResult final
  {
    ConnectionResult(ENetPeer* peer_, ConnectionStatus status_) :
      peer(peer_), status(status_)
    {}

    ENetPeer* const peer;
    ConnectionStatus status;
  };

public:
  Client(size_t outgoing_connections, size_t channel_limit,
         enet_uint32 incoming_bandwidth = 0, enet_uint32 outgoing_bandwidth = 0);

  ConnectionResult connect(const char* hostname, enet_uint16 port, size_t channel_count);
  void disconnect(ENetPeer* peer);

private:
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
};

} // namespace network

#endif

/* EOF */
