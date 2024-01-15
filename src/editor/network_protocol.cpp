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

#include "editor/network_protocol.hpp"

#include <ctime>

#include "editor/editor.hpp"
#include "editor/network_user.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

static void refresh_editor_menu()
{
  if (MenuManager::instance().is_active())
  {
    auto* editor_menu = dynamic_cast<EditorMenu*>(MenuManager::instance().current_menu());
    if (editor_menu)
      editor_menu->refresh();
  }
}

static const std::string DEFAULT_SERVER_NICKNAME = "Host";

EditorNetworkProtocol::EditorNetworkProtocol(Editor& editor, network::Host& host) :
  m_editor(editor),
  m_host(host)
{
}

void
EditorNetworkProtocol::on_server_connect(network::Peer& peer)
{
  refresh_editor_menu();

  // Create the editor user
  auto user = std::make_unique<EditorNetworkUser>(std::to_string(std::time(nullptr))); // TEMP
  peer.enet.data = user.get();

  // Notify all other peers of the newly connected user
  network::StagedPacket packet(OP_USER_SERVER_CONNECT, user->serialize());
  m_host.broadcast_packet_except(&peer.enet, packet, true);

  // Register the editor user
  m_editor.m_network_users.push_back(std::move(user));
}

void
EditorNetworkProtocol::on_server_disconnect(network::Peer& peer)
{
  refresh_editor_menu();

  // Get editor user
  assert(peer.enet.data);
  EditorNetworkUser* user = static_cast<EditorNetworkUser*>(peer.enet.data);

  // Notify all other peers of the disconnect user
  network::StagedPacket packet(OP_USER_SERVER_DISCONNECT, user->nickname);
  m_host.broadcast_packet_except(&peer.enet, packet, true);

  // Remove the editor user
  auto it = m_editor.m_network_users.begin();
  while (it != m_editor.m_network_users.end())
  {
    if (it->get() == user)
    {
      m_editor.m_network_users.erase(it);
      return;
    }
  }
}

void
EditorNetworkProtocol::on_client_connect(const network::ConnectionResult& result)
{
  if (result.status != network::ConnectionStatus::SUCCESS)
    return;

  // Add server at the beginning of network users list
  m_editor.m_network_users.clear();
  m_editor.m_network_users.push_back(std::make_unique<EditorNetworkUser>(DEFAULT_SERVER_NICKNAME));
}

void
EditorNetworkProtocol::on_client_disconnect(network::Peer&)
{
  m_editor.m_quit_request = true;
  Dialog::show_message(_("Disconnected: The server is no longer reachable."));
}

bool
EditorNetworkProtocol::verify_packet(network::StagedPacket& packet) const
{
  if (packet.code < 0 || packet.code >= OP_END)
    return false;

  if (packet.code == OP_USER_SERVER_CONNECT || packet.code == OP_USER_SERVER_DISCONNECT)
    return true; // User connect/disconnect events don't need a username

  // If server, and a foreign packet isn't being broadcasted, the packet originates from an action on this server.
  // Set username as the default username for servers.
  if (!packet.foreign_broadcast && !packet.is_part_of_request() &&
      m_editor.is_hosting_level())
    packet.data.insert(packet.data.begin(), DEFAULT_SERVER_NICKNAME);

  return true;
}

uint8_t
EditorNetworkProtocol::get_packet_channel(const network::StagedPacket& packet) const
{
  switch (packet.code)
  {
    case OP_USERS_REQUEST:
    case OP_USERS_RESPONSE:
    case OP_USER_SERVER_CONNECT:
    case OP_USER_SERVER_DISCONNECT:
      return CH_USER_SERVER_CONNECTIONS;

    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    case OP_LEVEL_RESPONSE:
    case OP_LEVEL_RERESPONSE:
      return CH_LEVEL_REQUESTS;

    case OP_SECTOR_CHANGES:
    case OP_SECTOR_CREATE:
    case OP_SECTOR_DELETE:
      return CH_SECTOR_CHANGES;

    case OP_MOUSE_CURSOR_UPDATE:
      return CH_MOUSE_UPDATES;

    default:
      return 0;
  }
}

void
EditorNetworkProtocol::on_packet_abort(network::RecievedPacket packet)
{
  switch (packet.code)
  {
    case OP_SECTOR_CHANGES:
      log_warning << "Failed to send sector changes." << std::endl;
      break;

    default:
      break;
  }
}

void
EditorNetworkProtocol::on_packet_recieve(network::RecievedPacket packet)
{
  /** Remote server connection */
  switch (packet.code)
  {
    case OP_USER_SERVER_CONNECT:
    {
      std::istringstream stream(packet.data[0]);
      auto doc = ReaderDocument::from_stream(stream);
      auto root = doc.get_root();
      if (root.get_name() != "supertux-editor-network-user")
        throw std::runtime_error("Cannot parse editor network user: Data is not 'supertux-editor-network-user'.");

      m_editor.m_network_users.push_back(std::make_unique<EditorNetworkUser>(root.get_mapping()));
      return;
    }
    case OP_USER_SERVER_DISCONNECT:
    {
      auto it = m_editor.m_network_users.begin();
      while (it != m_editor.m_network_users.end())
      {
        if ((*it)->nickname == packet.data[0])
        {
          m_editor.m_network_users.erase(it);
          return;
        }
      }
      return;
    }
  }

  /** Operations from existing user */
  EditorNetworkUser* user;
  if (m_editor.is_hosting_level()) // Server: Get user via peer userdata. Broadcast to all other peers.
  {
    assert(packet.peer && packet.peer->enet.data);
    user = static_cast<EditorNetworkUser*>(packet.peer->enet.data);

    network::StagedPacket broadcasted_packet(packet);
    broadcasted_packet.data.insert(broadcasted_packet.data.begin(), user->nickname);
    m_host.broadcast_packet_except(&packet.peer->enet, broadcasted_packet, true);
  }
  else // Client: Get user from editor users list.
  {
    user = m_editor.get_network_user(packet.data[0]);
    if (!user)
      throw std::runtime_error("Unknown user nickname: '" + packet.data[0] + "'.");

    packet.data.erase(packet.data.begin()); // Remove user nickname
  }

  switch (packet.code)
  {
    case OP_SECTOR_CHANGES:
    {
      std::istringstream stream(packet.data[0]);
      auto doc = ReaderDocument::from_stream(stream);
      auto root = doc.get_root();
      if (root.get_name() != "supertux-sector-changes")
      {
        log_warning << "Ignoring incoming sector changes data: Not 'supertux-sector-changes' data." << std::endl;
        return;
      }

      auto reader = root.get_mapping();

      std::string sector_name;
      reader.get("sector", sector_name);

      Sector* sector = m_editor.get_level()->get_sector(sector_name);
      if (!sector)
      {
        log_warning << "Ignoring incoming sector changes data: No sector with name '" << sector_name
                    << "' found." << std::endl;
        return;
      }

      BIND_SECTOR(*sector);
      sector->parse_properties(reader);

      std::optional<ReaderMapping> object_changes_mapping;
      if (reader.get("object-changes", object_changes_mapping))
      {
        GameObjectStates states(*object_changes_mapping);
        sector->apply_object_states(states);
      }
      break;
    }

    case OP_SECTOR_CREATE:
    {
      m_editor.create_sector(packet.data[0], true);
      break;
    }
    case OP_SECTOR_DELETE:
    {
      m_editor.delete_sector(packet.data[0], true);
      break;
    }

    case OP_MOUSE_CURSOR_UPDATE:
    {
      std::istringstream stream(packet.data[1]);
      auto doc = ReaderDocument::from_stream(stream);
      auto root = doc.get_root();
      if (root.get_name() != "supertux-mouse-cursor-state")
        throw std::runtime_error("Cannot parse editor network cursor update: Data is not 'supertux-mouse-cursor-state'.");

      user->sector = packet.data[0];
      user->mouse_cursor.parse_state(root.get_mapping());
      break;
    }

    default:
      break;
  }
}

network::StagedPacket
EditorNetworkProtocol::on_request_recieve(const network::RecievedPacket& packet)
{
  switch (packet.code) 
  {
    case OP_USERS_REQUEST:
    {
      assert(packet.peer && packet.peer->enet.data);
      return network::StagedPacket(OP_USERS_RESPONSE,
                                   m_editor.save_network_users(static_cast<EditorNetworkUser*>(packet.peer->enet.data)));
    }

    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    {
      if (!m_editor.get_level())
        throw std::runtime_error("No active editor level.");

      GameObject::s_save_uid = true;
      const std::string level_data = m_editor.get_level()->save();
      GameObject::s_save_uid = false;

      return network::StagedPacket(packet.code == OP_LEVEL_REQUEST ? OP_LEVEL_RESPONSE : OP_LEVEL_RERESPONSE,
                                   level_data, 10.f);
    }

    default:
      throw std::runtime_error("Invalid request code.");
  }
}

void
EditorNetworkProtocol::on_request_fail(const network::Request& request, network::Request::FailReason reason)
{
  std::string fail_reason;
  switch (reason)
  {
    case network::Request::FailReason::REQUEST_TIMED_OUT:
      fail_reason = _("Cannot send request: Timed out.");
      break;
    case network::Request::FailReason::RESPONSE_TIMED_OUT:
      fail_reason = _("Did not recieve response: Timed out.");
      break;
  }

  const network::StagedPacket& packet = *request.staged;
  switch (packet.code)
  {
    case OP_USERS_REQUEST:
    {
      m_editor.m_quit_request = true;
      Dialog::show_message(_("Disconnected: Network users request failed:") + "\n \n" + fail_reason);
      break;
    }

    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    {
      m_editor.m_quit_request = true;
      Dialog::show_message(_("Disconnected: Remote level request failed:") + "\n \n" + fail_reason);
      break;
    }

    default:
      break;
  }
}

void
EditorNetworkProtocol::on_request_response(const network::Request& request)
{
  const network::RecievedPacket& packet = *request.recieved;
  switch (packet.code)
  {
    case OP_USERS_RESPONSE:
      m_editor.parse_network_users(packet.data[0]);
      break;

    case OP_LEVEL_RESPONSE:
    case OP_LEVEL_RERESPONSE:
      m_editor.set_level(packet.data[0], packet.code == OP_LEVEL_RESPONSE, true);
      break;

    default:
      break;
  }
}

/* EOF */
