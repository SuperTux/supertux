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

#include "network/client.hpp"

#include <cassert>
#include <stdexcept>

namespace network {

Client::Client(size_t outgoing_connections, size_t channel_limit,
               enet_uint32 incoming_bandwidth, enet_uint32 outgoing_bandwidth) :
  Host()
{
  m_host = enet_host_create(NULL, outgoing_connections, channel_limit,
                            incoming_bandwidth, outgoing_bandwidth);
  if (!m_host)
    throw std::runtime_error("Error initializing ENet client!");
}

Client::ConnectionResult
Client::connect(const char* hostname, enet_uint16 port, size_t channel_count)
{
  ENetAddress address;
  enet_address_set_host(&address, hostname);
  address.port = port;

  // Initiate the connection
  ENetPeer* peer = enet_host_connect(m_host, &address, channel_count, 0);
  if (!peer)
    return ConnectionResult(nullptr, ConnectionStatus::FAILED_NO_PEERS);

  // Wait up to 5 seconds for the connection attempt to succeed
  ENetEvent event;
  if (enet_host_service(m_host, &event, 5000) == 0)
  {
    enet_peer_reset(peer);
    return ConnectionResult(nullptr, ConnectionStatus::FAILED_TIMED_OUT);
  }
  if (event.type != ENET_EVENT_TYPE_CONNECT)
  {
    enet_peer_reset(peer);
    return ConnectionResult(nullptr, ConnectionStatus::FAILED_DISCONNECT);
  }

  return ConnectionResult(peer, ConnectionStatus::SUCCESS);
}

void
Client::disconnect(ENetPeer* peer)
{
  if (!peer) return;
  assert(peer->host == m_host);

  enet_peer_disconnect(peer, 0);

  // Allow up to 3 seconds for the disconnect to succeed.
  // Drop any recieved packets.
  ENetEvent event;
  bool success = false;
  while (!success && enet_host_service(m_host, &event, 3000) > 0)
  {
    switch (event.type)
    {
      case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy(event.packet);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        success = true;
        break;
      default:
        break;
    }
  }

  // Force the connection down, if disconnecting failed
  if (!success)
    enet_peer_reset(peer);
}

} // namespace network

/* EOF */
