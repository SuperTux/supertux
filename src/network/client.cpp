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
#include <functional>
#include <stdexcept>

#include <version.h>

namespace network {

Client::Client(size_t outgoing_connections, size_t channel_limit,
               uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth) :
  Host()
{
  m_host = enet_host_create(NULL, outgoing_connections, channel_limit,
                            static_cast<enet_uint32>(incoming_bandwidth),
                            static_cast<enet_uint32>(outgoing_bandwidth));
  if (!m_host)
    throw std::runtime_error("Error initializing ENet client!");
}

void
Client::process_event(const ENetEvent& event)
{
  if (event.type == ENET_EVENT_TYPE_DISCONNECT)
  {
    Peer peer(*event.peer);
    m_protocol->on_client_disconnect(peer, static_cast<uint32_t>(event.data));

    // Reset the peer's client information
    event.peer->data = NULL;
  }
}

ConnectionResult
Client::connect(const char* hostname, uint16_t port,
                uint32_t wait_ms, size_t allocated_channels)
{
  auto result = connect_internal(hostname, port, wait_ms, allocated_channels);
  if (m_protocol)
    m_protocol->on_client_connect(result);
  return result;
}

ConnectionResult
Client::connect_internal(const char* hostname, uint16_t port,
                         uint32_t wait_ms, size_t allocated_channels)
{
  ENetAddress address;
  enet_address_set_host(&address, hostname);
  address.port = static_cast<enet_uint16>(port);

  // If a protocol is binded, use its required allocated channel count
  if (m_protocol)
    allocated_channels = m_protocol->get_channel_count();

  // Hash the game version
  std::hash<std::string> hasher;
  const enet_uint32 version = static_cast<enet_uint32>(hasher(PACKAGE_VERSION));

  // Initiate the connection
  ENetPeer* peer = enet_host_connect(m_host, &address, allocated_channels, version);
  if (!peer)
    return ConnectionResult(nullptr, ConnectionStatus::FAILED_NO_PEERS);

  // Wait up to 5 seconds for the connection attempt to succeed.
  // Return failure if disconnection events are fired.
  ENetEvent event;
  bool connected = false;
  while (enet_host_service(m_host, &event, static_cast<enet_uint32>(wait_ms)) > 0)
  {
    switch (event.type)
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        connected = true;
        break;
      }
      case ENET_EVENT_TYPE_DISCONNECT:
      {
        enet_peer_reset(peer);

        if (event.data == DISCONNECTED_VERSION_MISMATCH)
          return ConnectionResult(nullptr, ConnectionStatus::FAILED_VERSION_MISMATCH);

        return ConnectionResult(nullptr, ConnectionStatus::FAILED_CONNECTION_REFUSED);
      }
      default:
        break;
    }
  }

  // If no connection or disconnection event was fired, the connection has timed out
  if (!connected)
  {
    enet_peer_reset(peer);
    return ConnectionResult(nullptr, ConnectionStatus::FAILED_TIMED_OUT);
  }

  // Connection was successful!
  // Set peer properties and return result.
  enet_peer_ping_interval(peer, 10);
  return ConnectionResult(peer, ConnectionStatus::SUCCESS);
}

void
Client::disconnect(ENetPeer* peer)
{
  if (!peer) return;
  assert(peer->host == m_host);

  if (m_protocol)
  {
    Peer peer_info(*peer);
    m_protocol->on_client_disconnect(peer_info, DISCONNECTED_OK);
  }

  enet_peer_disconnect(peer, 0);

  // Allow up to 3 seconds for the disconnect to succeed.
  // Drop any received packets.
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
