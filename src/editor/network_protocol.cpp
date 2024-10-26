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

#include "editor/editor.hpp"
#include "editor/network_server_user.hpp"
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "network/client.hpp"
#include "network/server.hpp"
#include "supertux/d_scope.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

EditorNetworkProtocol::EditorNetworkProtocol(Editor& editor, network::Host& host) :
  network::UserProtocol(editor, host),
  m_editor(editor)
{
}

void
EditorNetworkProtocol::on_server_connect(network::Peer& peer)
{
  MenuManager::instance().refresh_menu<EditorMenu>();

  network::UserProtocol::on_server_connect(peer);
}

void
EditorNetworkProtocol::on_server_disconnect(network::Peer& peer, uint32_t code)
{
  MenuManager::instance().refresh_menu<EditorMenu>();

  network::UserProtocol::on_server_disconnect(peer, code);
}

void
EditorNetworkProtocol::on_client_disconnect(network::Peer&, uint32_t code)
{
  switch (code)
  {
    case network::DISCONNECTED_OK:
      break;

    case network::DISCONNECTED_PING_TIMEOUT:
      Dialog::show_message(_("Disconnected: The server is no longer reachable."));
      break;

    case network::DISCONNECTED_SERVER_CLOSED:
      Dialog::show_message(_("Disconnected: The server was closed."));
      break;

    case network::DISCONNECTED_NOT_WHITELISTED:
      Dialog::show_message(_("Disconnected: You are not whitelisted on the server."));
      break;

    case network::DISCONNECTED_KICKED:
      Dialog::show_message(_("Disconnected: You have been kicked from the server."));
      break;

    case network::DISCONNECTED_BANNED:
      Dialog::show_message(_("Disconnected: You have been banned from the server."));
      break;

    case DISCONNECTED_REGISTER_TIMED_OUT:
      Dialog::show_message(_("Disconnected: No registration packet received for too long."));
      break;

    case DISCONNECTED_NICKNAME_INVALID:
      Dialog::show_message(_("Disconnected: The provided nickname is invalid."));
      break;

    case DISCONNECTED_NICKNAME_TAKEN:
      Dialog::show_message(_("Disconnected: The provided nickname has been taken."));
      break;

    default:
      Dialog::show_message(_("Disconnected: Unknown reason."));
      break;
  }

  m_editor.reset_level();

  // Just destroy the client, since we are already disconnected from the server
  m_editor.m_network_client->destroy();
  m_editor.m_network_client = nullptr;
  m_editor.m_network_server_peer = nullptr;
}

bool
EditorNetworkProtocol::verify_packet(network::StagedPacket& packet) const
{
  if (!network::UserProtocol::verify_packet(packet))
    return false;

  if (packet.code < 0 || packet.code >= OP_END)
    return false;

  return true;
}

uint8_t
EditorNetworkProtocol::get_packet_channel(const network::StagedPacket& packet) const
{
  const uint8_t channel = network::UserProtocol::get_packet_channel(packet);
  if (channel != 0)
    return channel;

  switch (packet.code)
  {
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
EditorNetworkProtocol::on_packet_abort(network::ReceivedPacket packet)
{
  switch (packet.code)
  {
    case OP_SECTOR_CHANGES:
      log_warning << "Failed to send sector changes!" << std::endl;
      break;

    default:
      break;
  }
}

void
EditorNetworkProtocol::on_user_packet_receive(const network::ReceivedPacket& packet, network::ServerUser& user)
{
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
        GameObjectChangeSet change_set(*object_changes_mapping);
        sector->apply_object_changes(change_set, false); // Do not track remote object changes
      }
      break;
    }

    case OP_SECTOR_CREATE:
    {
      m_editor.add_sector(packet.data[0]);
      break;
    }
    case OP_SECTOR_DELETE:
    {
      m_editor.delete_sector(packet.data[0], true);
      break;
    }

    case OP_MOUSE_CURSOR_UPDATE:
    {
      auto doc = ReaderDocument::from_string(packet.data[1], "mouse-cursor-state");
      auto root = doc.get_root();
      if (root.get_name() != "supertux-mouse-cursor-state")
        throw std::runtime_error("Cannot parse editor network cursor update: Data is not 'supertux-mouse-cursor-state'.");

      EditorServerUser* editor_user = static_cast<EditorServerUser*>(&user);
      editor_user->sector = packet.data[0];
      editor_user->mouse_cursor.parse_state(root.get_mapping());
      break;
    }

    default:
      break;
  }
}

network::StagedPacket
EditorNetworkProtocol::on_request_receive(const network::ReceivedPacket& packet)
{
  try
  {
    return network::UserProtocol::on_request_receive(packet);
  }
  catch (...)
  {
  }

  switch (packet.code)
  {
    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    {
      if (!m_editor.get_level())
        throw std::runtime_error("No active editor level.");

      GameObject::s_save_uid = true;
      const std::string level_data = m_editor.get_level()->save();
      GameObject::s_save_uid = false;

      int code = packet.code == OP_LEVEL_REQUEST ? OP_LEVEL_RESPONSE : OP_LEVEL_RERESPONSE;
      if (m_editor.get_level()->is_worldmap())
        code = packet.code == OP_LEVEL_REQUEST ? OP_WORLDMAP_RESPONSE : OP_WORLDMAP_RERESPONSE;

      return network::StagedPacket(code, level_data, 10.f);
    }

    default:
      throw std::runtime_error("Invalid request code!");
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
      fail_reason = _("Did not receive response: Timed out.");
      break;
  }

  const network::StagedPacket& packet = *request.staged;
  switch (packet.code)
  {
    case OP_USER_REGISTER:
    {
      Dialog::show_message(_("Disconnected: Registration request failed:") + "\n \n" + fail_reason);
      m_editor.close_connections();
      break;
    }

    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    {
      Dialog::show_message(_("Disconnected: Remote level request failed:") + "\n \n" + fail_reason);
      m_editor.m_quit_request = true;
      break;
    }

    default:
      break;
  }
}

void
EditorNetworkProtocol::on_request_response(const network::Request& request)
{
  network::UserProtocol::on_request_response(request);

  const network::ReceivedPacket& packet = *request.received;
  switch (packet.code)
  {
    case OP_USER_REGISTER: /** This client has been registered on the remote server. */
    {
      // Request the level from the remote server.
      m_host.send_request(m_editor.m_network_server_peer,
                          std::make_unique<network::Request>(
                            std::make_unique<network::StagedPacket>(OP_LEVEL_REQUEST, "", 10.f),
                            12.f));
      break;
    }

    case OP_LEVEL_RESPONSE:
    case OP_LEVEL_RERESPONSE:
    case OP_WORLDMAP_RESPONSE:
    case OP_WORLDMAP_RERESPONSE:
    {
      m_editor.set_world({});
      m_editor.set_level(packet.data[0],
                         packet.code == OP_LEVEL_RESPONSE || packet.code == OP_WORLDMAP_RESPONSE, true,
                         packet.code == OP_WORLDMAP_RESPONSE || packet.code == OP_WORLDMAP_RERESPONSE);
      break;
    }

    default:
      break;
  }
}

/* EOF */
