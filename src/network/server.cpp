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

#include <algorithm>
#include <functional>
#include <stdexcept>

#include <version.h>

#include "gui/menu_manager.hpp"
#include "network/connection_result.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"
#include "supertux/menu/server_management_menu.hpp"
#include "util/gettext.hpp"
#include "util/log.hpp"

namespace network {

Server::Server(uint16_t port, size_t peer_count, size_t channel_limit,
               uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth) :
  Host()
{
  if (port != 0 && port < 1024)
    throw std::runtime_error(_("Port number is less than 1024."));

  ENetAddress address;
  address.host = ENET_HOST_ANY;
  address.port = static_cast<enet_uint16>(port);

  m_host = enet_host_create(&address, peer_count, channel_limit,
                            static_cast<enet_uint32>(incoming_bandwidth),
                            static_cast<enet_uint32>(outgoing_bandwidth));
  if (!m_host)
    throw std::runtime_error(_("Error initializing ENet server!"));
}

Server::~Server()
{
  // Disconnect all connected peers, notifying them that the server was closed
  for (ENetPeer* peer = m_host->peers; peer < &m_host->peers[m_host->peerCount]; peer++)
  {
    if (peer->state != ENET_PEER_STATE_CONNECTED) continue;

    enet_peer_disconnect_now(peer, DISCONNECTED_SERVER_CLOSED);

    // Reset the peer's client information
    peer->data = NULL;
  }
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
        enet_peer_disconnect_now(event.peer, DISCONNECTED_VERSION_MISMATCH);
        return;
      }

      // Make sure the peer is not banned/is whitelisted (depending on restriction mode)
      Peer peer(*event.peer);
      if (g_config->network_restrict_mode == Config::NetworkRestrictMode::BLACKLIST &&
          std::find(g_config->network_blacklist.begin(), g_config->network_blacklist.end(),
                    peer.address.host) != g_config->network_blacklist.end())
      {
        log_warning << "Banned peer '" << peer.address
                    << "' attempted to connect to server. Disconnecting." << std::endl;
        enet_peer_disconnect_now(event.peer, DISCONNECTED_BANNED);
        return;
      }
      else if (g_config->network_restrict_mode == Config::NetworkRestrictMode::WHITELIST &&
               std::find(g_config->network_whitelist.begin(), g_config->network_whitelist.end(),
                         peer.address.host) == g_config->network_whitelist.end())
      {
        log_warning << "Non-whitelisted peer '" << peer.address
                    << "' attempted to connect to server. Disconnecting." << std::endl;
        enet_peer_disconnect_now(event.peer, DISCONNECTED_NOT_WHITELISTED);
        return;
      }

      if (m_protocol)
        m_protocol->on_server_connect(peer);

      MenuManager::instance().refresh_menu<ServerManagementMenu>();
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT:
    {
      Peer peer(*event.peer);
      if (m_protocol)
        m_protocol->on_server_disconnect(peer, static_cast<uint32_t>(event.data));

      // Reset the peer's client information
      event.peer->data = NULL;

      MenuManager::instance().refresh_menu<ServerManagementMenu>();
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

  if (code == DISCONNECTED_KICKED)
    kick(peer);
  else if (code == DISCONNECTED_BANNED)
    ban(peer);
  else
    enet_peer_disconnect(peer, static_cast<enet_uint32>(code));
}

void
Server::kick(ENetPeer* peer)
{
  if (!peer) return;
  assert(peer->host == m_host);

  enet_peer_disconnect_now(peer, DISCONNECTED_KICKED);

  // No ENET_EVENT_TYPE_DISCONNECT will be generated, so notify protocol directly from here
  if (m_protocol)
  {
    Peer peer_info(*peer);
    m_protocol->on_server_disconnect(peer_info, DISCONNECTED_KICKED);
  }

  // Reset the peer's client information
  peer->data = NULL;

  MenuManager::instance().refresh_menu<ServerManagementMenu>();
}

void
Server::ban(ENetPeer* peer)
{
  if (!peer) return;
  assert(peer->host == m_host);

  // Save banned host in config
  Address ban_address(peer->address);
  g_config->network_blacklist.push_back(ban_address.host);

  // Disconnect every peer from the provided peer's host address
  for (ENetPeer* it_peer = m_host->peers; it_peer < &m_host->peers[m_host->peerCount]; it_peer++)
  {
    Peer peer_info(*it_peer);
    if (it_peer->state == ENET_PEER_STATE_CONNECTED &&
        peer_info.address.host == ban_address.host)
    {
      enet_peer_disconnect_now(it_peer, DISCONNECTED_BANNED);

      // No ENET_EVENT_TYPE_DISCONNECT will be generated, so notify protocol directly from here
      if (m_protocol)
        m_protocol->on_server_disconnect(peer_info, DISCONNECTED_BANNED);

      // Reset the peer's client information
      it_peer->data = NULL;
    }
  }

  MenuManager::instance().refresh_menu<ServerManagementMenu>();
}

std::vector<RemoteUser>
Server::get_users() const
{
  std::vector<RemoteUser> result;

  for (ENetPeer* peer = m_host->peers; peer < &m_host->peers[m_host->peerCount]; peer++)
  {
    if (peer->state != ENET_PEER_STATE_CONNECTED) continue;

    RemoteUser user(*peer);
    if (m_protocol) // Request user data from the protocol
    {
      try
      {
        m_protocol->get_remote_user_data(user);
      }
      catch (const std::exception& err)
      {
        log_warning << "Error getting user from peer " << peer->incomingPeerID
                    << " in protocol: " << err.what() << std::endl;
      }
    }
    result.push_back(std::move(user));
  }

  return result;
}

Address
Server::get_address() const
{
  return Address(m_host->address);
}

} // namespace network

/* EOF */
