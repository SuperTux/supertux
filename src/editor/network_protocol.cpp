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
#include "gui/dialog.hpp"
#include "gui/menu_manager.hpp"
#include "supertux/level.hpp"
#include "supertux/menu/editor_menu.hpp"
#include "util/log.hpp"

static void refresh_editor_menu()
{
  if (MenuManager::instance().is_active())
  {
    auto* editor_menu = dynamic_cast<EditorMenu*>(MenuManager::instance().current_menu());
    if (editor_menu)
      editor_menu->refresh();
  }
}


EditorNetworkProtocol::EditorNetworkProtocol(Editor& editor) :
  m_editor(editor)
{
}

void
EditorNetworkProtocol::on_server_connect(network::Peer&)
{
  refresh_editor_menu();
}

void
EditorNetworkProtocol::on_server_disconnect(network::Peer&)
{
  refresh_editor_menu();
}

void
EditorNetworkProtocol::on_client_disconnect(network::Peer&)
{
  m_editor.m_quit_request = true;
  Dialog::show_message(_("Disconnected: The server is no longer reachable."));
}

bool
EditorNetworkProtocol::verify_packet(const network::StagedPacket& packet)
{
  if (packet.code < 0 || packet.code >= OP_END)
    return false;

  return true;
}

network::StagedPacket
EditorNetworkProtocol::on_request_recieve(const network::RecievedPacket& packet)
{
  switch (packet.code)
  {
    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
      if (!m_editor.get_level()) return {};
      return network::StagedPacket(packet.code == OP_LEVEL_REQUEST ? OP_LEVEL_RESPONSE : OP_LEVEL_RERESPONSE,
                                   m_editor.get_level()->save());

    default:
      return {};
  }
}

void
EditorNetworkProtocol::on_request_fail(const network::Request& request, network::Request::FailReason reason)
{
  const network::StagedPacket& packet = *request.staged;
  switch (packet.code)
  {
    case OP_LEVEL_REQUEST:
    case OP_LEVEL_REREQUEST:
    {
      m_editor.m_quit_request = true;

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
    case OP_LEVEL_RESPONSE:
    case OP_LEVEL_RERESPONSE:
      m_editor.set_level(packet.data, packet.code == OP_LEVEL_RESPONSE, true);
      break;

    default:
      break;
  }
}

/* EOF */
