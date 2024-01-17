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

#include "network/server.hpp"

#include <functional>
#include <stdexcept>

#include <version.h>

#include "network/connection_result.hpp"

namespace network {

Server::Server(uint16_t port, size_t peer_count, size_t channel_limit,
               uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth) :
  Host()
{
  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = static_cast<enet_uint16>(port);

  m_host = enet_host_create(&address, peer_count, channel_limit,
                            static_cast<enet_uint32>(incoming_bandwidth),
                            static_cast<enet_uint32>(outgoing_bandwidth));
  if (!m_host)
    throw std::runtime_error("Error initializing ENet server!");
}

void
Server::process_event(const ENetEvent& event)
{
  switch (event.type)
  {
    case ENET_EVENT_TYPE_CONNECT:
    {
      // Hash the game version
      std::hash<std::string> hasher;
      const enet_uint32 version = static_cast<enet_uint32>(hasher(PACKAGE_VERSION));

      // Make sure remote peer game version is the same
      if (event.data != version)
      {
        enet_peer_disconnect(event.peer, DISCONNECTED_VERSION_MISMATCH);
        return;
      }

      Peer peer(*event.peer);
      m_protocol->on_server_connect(peer);
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT:
    {
      Peer peer(*event.peer);
      m_protocol->on_server_disconnect(peer);

      // Reset the peer's client information
      event.peer->data = NULL;
      break;
    }
    default:
      break;
  }
}

void
Server::disconnect(ENetPeer* peer, uint32_t code)
{
  if (!peer) return;
  assert(peer->host == m_host);

  enet_peer_disconnect(peer, static_cast<enet_uint32>(code));
}

Address
Server::get_address() const
{
  return Address(m_host->address);
}

} // namespace network

/* EOF */
