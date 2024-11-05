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

#include "network/user_protocol.hpp"

#include "gui/menu_manager.hpp"
#include "network/server.hpp"
#include "network/server_user.hpp"
#include "network/user_manager.hpp"
#include "supertux/menu/server_management_menu.hpp"
#include "supertux/sector.hpp"
#include "supertux/timer.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

namespace network {

static const float USER_REGISTER_TIME = 5.f;

template<class U>
bool
UserProtocol<U>::verify_username(const std::string& username)
{
  // Must be between 3 and 20 characters.
  if (username.length() < 3 || username.length() > 20)
    return false;

  return true;
}


template<class U>
UserProtocol<U>::UserProtocol(UserManager<U>& user_manager, Host& host) :
  m_user_manager(user_manager),
  m_host(host),
  m_pending_users()
{
  assert(m_user_manager.m_self_user);
}

template<class U>
void
UserProtocol<U>::update()
{
  // Remove any timed-out pending (non-registered) peers
  {
    auto it = m_pending_users.begin();
    while (it != m_pending_users.end())
    {
      if (it->second->check())
      {
        Server& server = static_cast<Server&>(m_host);
        server.disconnect(it->first, DISCONNECTED_REGISTER_TIMED_OUT);

        it = m_pending_users.erase(it);
      }
      else
        it++;
    }
  }
}

template<class U>
void
UserProtocol<U>::on_server_connect(Peer& peer)
{
  // Stage the user and wait for registration packet
  auto timer = std::make_unique<Timer>();
  timer->start(USER_REGISTER_TIME);
  m_pending_users[&peer.enet] = std::move(timer);
}

template<class U>
void
UserProtocol<U>::on_server_disconnect(Peer& peer, uint32_t)
{
  // Get server user (if it has been registered)
  if (peer.enet.data)
  {
    U* user = static_cast<U*>(peer.enet.data);
    on_user_disconnect(*user);

    // Notify all other peers of the disconnect user
    StagedPacket packet(OP_USER_SERVER_DISCONNECT, user->username);
    m_host.broadcast_packet(packet, true, &peer.enet);

    // Remove the server user
    auto it = m_user_manager.m_server_users.begin();
    while (it != m_user_manager.m_server_users.end())
    {
      if (it->get() == user)
      {
        m_user_manager.m_server_users.erase(it);
        break;
      }
      it++;
    }
  }
}

template<class U>
bool
UserProtocol<U>::allow_packet_send(Peer& peer) const
{
  // Do not allow sending packets to any unregistered peers
  auto it = m_pending_users.begin();
  while (it != m_pending_users.end())
  {
    if (it->first == &peer.enet)
      return false;

    it++;
  }

  return true;
}

template<class U>
bool
UserProtocol<U>::verify_packet(StagedPacket& packet) const
{
  if (packet.code == OP_USER_SERVER_CONNECT || packet.code == OP_USER_SERVER_DISCONNECT)
    return true; // User connect/disconnect events don't need a username

  // If server, and a foreign packet isn't being broadcasted, the packet originates from an action on this server.
  // Set username as the default username for servers.
  if (!packet.foreign_broadcast && !packet.is_part_of_request() &&
      m_host.is_server())
    packet.data.insert(packet.data.begin(), m_user_manager.m_self_user->username);

  return true;
}

template<class U>
uint8_t
UserProtocol<U>::get_packet_channel(const StagedPacket& packet) const
{
  switch (packet.code)
  {
    case OP_USER_REGISTER:
    case OP_USER_SERVER_CONNECT:
    case OP_USER_SERVER_DISCONNECT:
      return CH_USER_SERVER_CONNECTIONS;

    default:
      return 0;
  }
}

template<class U>
void
UserProtocol<U>::on_packet_receive(ReceivedPacket packet)
{
  /** Remote server connection */
  switch (packet.code)
  {
    case OP_USER_SERVER_CONNECT:
    {
      auto doc = ReaderDocument::from_string(packet.data[0], "server-user");
      auto root = doc.get_root();
      if (root.get_name() != "supertux-server-user")
        throw std::runtime_error("Cannot parse server user: Data is not 'supertux-server-user'.");

      auto user = std::make_unique<U>(root.get_mapping());
      on_user_connect(*user);
      m_user_manager.m_server_users.push_back(std::move(user));
      return;
    }
    case OP_USER_SERVER_DISCONNECT:
    {
      auto it = m_user_manager.m_server_users.begin();
      while (it != m_user_manager.m_server_users.end())
      {
        if ((*it)->username == packet.data[0])
        {
          on_user_disconnect(**it);
          m_user_manager.m_server_users.erase(it);
          break;
        }
        it++;
      }
      return;
    }
  }

  /** Operations from existing user */
  U* user;
  if (m_host.is_server()) // Server: Get user via peer userdata. Broadcast to all other peers.
  {
    assert(packet.peer && packet.peer->enet.data);
    user = static_cast<U*>(packet.peer->enet.data);

    if (on_user_packet_receive(packet, *user))
    {
      StagedPacket broadcasted_packet(packet);
      broadcasted_packet.data.insert(broadcasted_packet.data.begin(), user->username);
      m_host.broadcast_packet(broadcasted_packet, true, &packet.peer->enet);
    }
  }
  else // Client: Get user from users list.
  {
    user = m_user_manager.get_server_user(packet.data[0]);
    if (!user)
      throw std::runtime_error("Unknown user username: '" + packet.data[0] + "'.");

    packet.data.erase(packet.data.begin()); // Remove user username

    on_user_packet_receive(packet, *user);
  }
}

template<class U>
StagedPacket
UserProtocol<U>::on_request_receive(const ReceivedPacket& packet)
{
  switch (packet.code)
  {
    case OP_USER_REGISTER:
    {
      if (!m_host.is_server())
        throw std::runtime_error("Cannot perform user registration on non-server!");

      if (packet.peer->enet.data)
      {
        const ServerUser* user = static_cast<ServerUser*>(packet.peer->enet.data);
        throw std::runtime_error("User \"" + user->username + "\" tried to register again!");
      }

      // Parse the server user
      auto doc = ReaderDocument::from_string(packet.data[0], "server-user");
      auto root = doc.get_root();
      if (root.get_name() != "supertux-server-user")
        throw std::runtime_error("Cannot parse server user: Data is not 'supertux-server-user'.");

      auto reader = root.get_mapping();
      std::string username;
      reader.get("username", username);

      // Check if username is valid
      if (!verify_username(username))
      {
        Server& server = static_cast<Server&>(m_host);
        server.disconnect(&packet.peer->enet, DISCONNECTED_USERNAME_INVALID);

        throw std::runtime_error("Cannot register user: Provided username is invalid.");
      }

      // Check if username is taken
      if (m_user_manager.m_self_user->username == username)
      {
        Server& server = static_cast<Server&>(m_host);
        server.disconnect(&packet.peer->enet, DISCONNECTED_USERNAME_TAKEN);

        throw std::runtime_error("Cannot register user: Provided username taken.");
      }
      for (const auto& user : m_user_manager.m_server_users)
      {
        if (user->username == username)
        {
          Server& server = static_cast<Server&>(m_host);
          server.disconnect(&packet.peer->enet, DISCONNECTED_USERNAME_TAKEN);

          throw std::runtime_error("Cannot register user: Provided username taken.");
        }
      }

      // Create the server user
      auto user = std::make_unique<U>(reader);
      packet.peer->enet.data = user.get();

      on_user_connect(*user);

      // Notify all other peers of the newly connected user
      StagedPacket staged_packet(OP_USER_SERVER_CONNECT, user->serialize());
      m_host.broadcast_packet(staged_packet, true, &packet.peer->enet);

      // Add to users list
      m_user_manager.m_server_users.push_back(std::move(user));
      MenuManager::instance().refresh_menu<ServerManagementMenu>();

      // Remove peer from pending users
      m_pending_users.erase(&packet.peer->enet);

      // Indicate success. Return a list of all registered users.
      return StagedPacket(OP_USER_REGISTER, m_user_manager.save_server_users(static_cast<ServerUser*>(packet.peer->enet.data)));
    }

    default:
    {
      if (m_host.is_server()) // Server: Get user via peer userdata.
      {
        assert(packet.peer && packet.peer->enet.data);
        return on_server_user_request_receive(packet, *static_cast<U*>(packet.peer->enet.data));
      }

      throw std::runtime_error("Invalid request code!");
    }
  }
}

template<class U>
network::StagedPacket
UserProtocol<U>::on_server_user_request_receive(const ReceivedPacket&, U&)
{
  throw std::runtime_error("Invalid request code!");
}

template<class U>
void
UserProtocol<U>::on_request_response(const Request& request)
{
  const ReceivedPacket& packet = *request.received;
  switch (packet.code)
  {
    case OP_USER_REGISTER: /** This client has been registered on the remote server. */
    {
      // Parse received server users.
      m_user_manager.parse_server_users(packet.data[0]);
      break;
    }

    default:
      break;
  }
}


template<class U>
void
UserProtocol<U>::get_remote_user_data(RemoteUser& user) const
{
  if (user.peer.enet.data) // The peer has been registered
    user.display_text = static_cast<ServerUser*>(user.peer.enet.data)->username;
  else
    user.display_text = "UNREGISTERED";

  user.display_text += " (" + user.peer.address.to_string() + ")";
}

} // namespace network


/** Explicit template instantiations */

#include "editor/network_server_user.hpp"
#include "supertux/game_network_server_user.hpp"

namespace network {

//template class UserProtocol<ServerUser>;
template class UserProtocol<EditorServerUser>;
template class UserProtocol<GameServerUser>;

} // namespace network

/* EOF */
