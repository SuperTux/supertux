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

#include "supertux/game_network_protocol.hpp"

#include "gui/dialog.hpp"
#include "network/client.hpp"
#include "network/server.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/levelintro.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"

GameNetworkProtocol::GameNetworkProtocol(GameManager& game_manager, network::Host& host) :
  network::UserProtocol<GameServerUser>(game_manager, host),
  m_game_manager(game_manager),
  m_network_game_session()
{
}

void
GameNetworkProtocol::update()
{
  network::UserProtocol<GameServerUser>::update();

  for (const auto& user : m_game_manager.get_server_users())
  {
    for (const auto& controller : user->player_controllers)
      controller->update();
  }
}

void
GameNetworkProtocol::on_user_connect(GameServerUser& user)
{
  if (!GameSession::current())
    return;

  for (int i = 0; i < user.get_num_players(); i++)
    GameSession::current()->spawn_remote_player(user, i);
}

void
GameNetworkProtocol::on_user_disconnect(GameServerUser& user)
{
  if (!GameSession::current())
    return;

  for (int i = user.get_num_players() - 1; i >= 0; i--)
    GameSession::current()->despawn_remote_player(user, i);
}

void
GameNetworkProtocol::on_client_disconnect(network::Peer&, uint32_t code)
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

    case DISCONNECTED_USERNAME_INVALID:
      Dialog::show_message(_("Disconnected: The provided username is invalid."));
      break;

    case DISCONNECTED_USERNAME_TAKEN:
      Dialog::show_message(_("Disconnected: The provided username has been taken."));
      break;

    default:
      Dialog::show_message(_("Disconnected: Unknown reason."));
      break;
  }

  // TODO: Close

  // Just destroy the client, since we are already disconnected from the server
  m_game_manager.m_network_client->destroy();
  m_game_manager.m_network_client = nullptr;
  m_game_manager.m_network_server_peer = nullptr;
}

bool
GameNetworkProtocol::verify_packet(network::StagedPacket& packet) const
{
  if (!network::UserProtocol<GameServerUser>::verify_packet(packet))
    return false;

  if (packet.code < 0 || packet.code >= OP_END)
    return false;

  return true;
}

uint8_t
GameNetworkProtocol::get_packet_channel(const network::StagedPacket& packet) const
{
  const uint8_t channel = network::UserProtocol<GameServerUser>::get_packet_channel(packet);
  if (channel != 0)
    return channel;

  switch (packet.code)
  {
    case OP_GAME_JOIN:
    case OP_GAME_START:
      return CH_GAME_JOIN_REQUESTS;

    case OP_PLAYER_ADD:
    case OP_PLAYER_REMOVE:
    case OP_PLAYER_SPAWN:
    case OP_PLAYER_DESPAWN:
      return CH_PLAYER_UPDATES;

    case OP_CONTROLLER_UPDATE:
      return CH_CONTROLLER_UPDATES;

    case OP_GAME_OBJECT_UPDATE:
      return CH_GAME_OBJECT_UPDATES;

    default:
      return 0;
  }
}

bool
GameNetworkProtocol::on_user_packet_receive(const network::ReceivedPacket& packet, GameServerUser& user)
{
  switch (packet.code)
  {
    case OP_GAME_JOIN:
    {
      if (m_host.is_server())
        throw std::runtime_error("Cannot process game join from \"" + user.username + "\": This host is a server.");

      m_network_game_session = m_game_manager.start_network_level(
        user.username,
        packet.data[0],
        packet.data[1],
        packet.data[2]
      );
      break;
    }

    case OP_GAME_START:
    {
      if (m_host.is_server())
        throw std::runtime_error("Cannot process game start from \"" + user.username + "\": This host is a server.");

      if (!m_network_game_session || !ScreenManager::current()->get_top_screen<LevelIntro>())
        throw std::runtime_error("Invalid game start packet received.");

      LevelIntro::quit();
      break;
    }

    case OP_PLAYER_ADD:
    {
      // TODO: Make sure user players are lower than max local players
      user.push_player();
      return true;
    }
    case OP_PLAYER_REMOVE:
    {
      if (user.get_num_players() <= 1)
        throw std::runtime_error("Cannot process player removal from \"" + user.username + "\": Attempted to remove the last player of this user.");

      if (GameSession::current())
        GameSession::current()->despawn_remote_player(user, user.get_num_players() - 1);

      user.pop_player();
      return true;
    }
    case OP_PLAYER_SPAWN:
    {
      if (m_host.is_server()) // Servers are supposed to process player spawns as Requests (so they can return info about target player).
        throw std::runtime_error("Cannot process player spawn from \"" + user.username + "\": This host is a server.");

      if (!GameSession::current())
        throw std::runtime_error("Cannot process player spawn from server: No active GameSession.");

      const GameServerUser* user = m_game_manager.get_server_user(packet.data[0]);
      if (!user)
        throw std::runtime_error("Cannot process player spawn from server: Unknown user username: '" + packet.data[0] + "'.");

      const int player_id = std::stoi(packet.data[1]);
      if (player_id < 0 || player_id >= user->get_num_players())
        throw std::runtime_error("Cannot process player spawn from \"" + user->username + "\": Player " + std::to_string(player_id + 1) + " does not exist.");

      const GameServerUser* target_user = m_game_manager.get_server_user(packet.data[2]);
      if (!target_user)
        throw std::runtime_error("Cannot process player spawn from \"" + user->username + "\": Unknown target user username: '" + packet.data[2] + "'.");

      const int target_player_id = std::stoi(packet.data[3]);
      if (target_player_id < 0 || target_player_id >= target_user->get_num_players())
        throw std::runtime_error("Cannot process player spawn from \"" + user->username +
            "\": Player " + std::to_string(target_player_id + 1) + " does not exist under user \"" + target_user->username + "\".");

      GameSession::current()->spawn_remote_player(*user, player_id, target_user, target_player_id);
      return true;
    }
    case OP_PLAYER_DESPAWN:
    {
      if (!GameSession::current())
        throw std::runtime_error("Cannot process player despawn from \"" + user.username + "\": No active GameSession.");

      const int player_id = std::stoi(packet.data[0]);
      if (player_id < 0 || player_id >= user.get_num_players())
        throw std::runtime_error("Cannot process player despawn from \"" + user.username + "\": Player " + std::to_string(player_id + 1) + " does not exist.");

      GameSession::current()->despawn_remote_player(user, player_id);
      return true;
    }

    case OP_CONTROLLER_UPDATE:
    {
      const int controller_user = std::stoi(packet.data[0]);
      if (controller_user >= static_cast<int>(user.get_num_players()))
        throw std::runtime_error("Cannot process controller update from \"" + user.username + "\": Remote controller user " + packet.data[0] + " doesn't exist.");

      user.player_controllers[controller_user]->process_packet_data(packet, 1);
      return true;
    }

    case OP_GAME_OBJECT_UPDATE:
    {
      // TODO
    }

    default:
      break;
  }

  return false;
}

network::StagedPacket
GameNetworkProtocol::on_server_user_request_receive(const network::ReceivedPacket& packet, GameServerUser& user)
{
  switch (packet.code)
  {
    case OP_PLAYER_SPAWN:
    {
      if (!GameSession::current())
        throw std::runtime_error("Cannot process player spawn request from \"" + user.username + "\": No active GameSession.");

      const int player_id = std::stoi(packet.data[0]);
      if (player_id < 0 || player_id >= user.get_num_players())
        throw std::runtime_error("Cannot process player spawn request from \"" + user.username + "\": Player " + std::to_string(player_id + 1) + " does not exist.");

      auto target_player = GameSession::current()->spawn_remote_player(user, player_id);
      if (!target_player.first)
        throw std::runtime_error("Cannot process player spawn request from \"" + user.username + "\": Target player user not available!");

      // Plater was spawned. Return info about its target player.
      return network::StagedPacket(OP_PLAYER_SPAWN, {
          target_player.first->username,
          std::to_string(target_player.second)
        }, 2.f);
    }

    default:
      throw std::runtime_error("Invalid request code!");
  }
}

void
GameNetworkProtocol::on_request_fail(const network::Request& request, network::Request::FailReason reason)
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
      // TODO: Close
      break;
    }

    default:
      break;
  }
}

void
GameNetworkProtocol::on_request_response(const network::Request& request)
{
  network::UserProtocol<GameServerUser>::on_request_response(request);

  const network::ReceivedPacket& packet = *request.received;
  switch (packet.code)
  {
    case OP_PLAYER_SPAWN:
    {
      if (m_host.is_server())
        throw std::runtime_error("Cannot process player spawn request response on non-server!");

      if (!GameSession::current())
        throw std::runtime_error("Cannot process player spawn request response from server: No active GameSession.");

      const GameServerUser* target_user = m_game_manager.get_server_user(packet.data[0]);
      if (!target_user)
        throw std::runtime_error("Cannot process player spawn request response from server: Unknown target user username: '" + packet.data[0] + "'.");

      const int target_player_id = std::stoi(packet.data[1]);
      if (target_player_id < 0 || target_player_id >= target_user->get_num_players())
        throw std::runtime_error("Cannot process player spawn request response from server: Player " +
            std::to_string(target_player_id + 1) + " does not exist on target user '" + target_user->username + "'.");

      GameSession::current()->spawn_local_player(std::stoi(request.staged->data[0]), target_user, target_player_id);
      break;
    }

    default:
      break;
  }
}

/* EOF */
