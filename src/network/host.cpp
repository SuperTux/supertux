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

#include "network/host.hpp"

#include <cassert>
#include <cstring>

#include "network/protocol.hpp"

namespace network {

Host::Host() :
  m_host(),
  m_protocol()
{
}

Host::~Host()
{
  assert(m_host);
  enet_host_destroy(m_host);
}

void
Host::update()
{
  /** Event handling on binded protocol */
  if (m_protocol)
  {
    ENetEvent event;
    while (enet_host_service(m_host, &event, 1000) > 0)
    {
      switch (event.type)
      {
        case ENET_EVENT_TYPE_CONNECT:
        {
          Peer peer(*event.peer);
          m_protocol->on_connect(peer);
          break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
          Peer peer(*event.peer);
          m_protocol->on_disconnect(peer);

          // Reset the peer's client information
          event.peer->data = NULL;
          break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
          m_protocol->on_packet_recieve(*event.packet);

          // Clean up the packet now that we're done using it
          enet_packet_destroy(event.packet);
          break;
        }
        default:
          break;
      }
    }
  }
}

void
Host::send_packet(ENetPeer* peer, const char* data, enet_uint8 channel_id)
{
  if (!peer) return;
  assert(peer->host == m_host);

  // Create a reliable packet containing the provided data
  ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);

  // Send the packet over
  enet_peer_send(peer, channel_id, packet);
  enet_host_flush(m_host);
}

} // namespace network

/* EOF */
