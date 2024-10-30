//  SuperTux
//  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/game_manager.hpp"

#include "control/input_manager.hpp"
#include "editor/editor.hpp"
#include "gui/dialog.hpp"
#include "network/client.hpp"
#include "network/host_manager.hpp"
#include "network/server.hpp"
#include "sdk/integration.hpp"
#include "supertux/game_network_protocol.hpp"
#include "supertux/game_session.hpp"
#include "supertux/levelset_screen.hpp"
#include "supertux/player_status.hpp"
#include "supertux/profile.hpp"
#include "supertux/savegame.hpp"
#include "supertux/screen.hpp"
#include "supertux/screen_fade.hpp"
#include "supertux/screen_manager.hpp"
#include "supertux/world.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"
#include "worldmap/worldmap_screen.hpp"

GameManager::GameManager() :
  m_savegame(),
  m_next_worldmap(),
  m_network_server(),
  m_network_client(),
  m_network_server_peer()
{
}

void
GameManager::start_level(const World& world, const std::string& level_filename,
                         const std::optional<std::pair<std::string, Vector>>& start_pos)
{
  m_savegame = Savegame::from_current_profile(world.get_basename());

  auto screen = std::make_unique<LevelsetScreen>(world.get_basedir(),
                                                 level_filename,
                                                 *m_savegame,
                                                 start_pos,
                                                 m_network_server);
  ScreenManager::current()->push_screen(std::move(screen));

  if (!Editor::current())
    m_savegame->get_profile().set_last_world(world.get_basename());
}

void
GameManager::start_worldmap_level(const std::string& level_filename, Savegame& savegame,
                                  Statistics* statistics)
{
  auto screen = std::make_unique<GameSession>(level_filename,
                                              savegame,
                                              statistics,
                                              false,
                                              std::nullopt,
                                              m_network_server);
  ScreenManager::current()->push_screen(std::move(screen));
}

GameSession*
GameManager::start_network_level(const std::string& level_content)
{
  m_savegame = std::make_unique<Savegame>();

  auto screen = std::make_unique<GameSession>(level_content,
                                              *m_savegame,
                                              nullptr,
                                              false,
                                              std::nullopt, // TODO: Receive first GameSession::SpawnPoint.
                                              m_network_client);
  GameSession* session = screen.get();
  ScreenManager::current()->push_screen(std::move(screen));
  return session;
}

void
GameManager::start_worldmap(const World& world, const std::string& worldmap_filename,
                            const std::string& sector, const std::string& spawnpoint)
{
  try
  {
    m_savegame = Savegame::from_current_profile(world.get_basename());

    auto filename = m_savegame->get_player_status().last_worldmap;
    // If we specified a worldmap filename manually,
    // this overrides the default choice of "last worldmap".
    if (!worldmap_filename.empty())
    {
      filename = worldmap_filename;
    }

    // No "last worldmap" found and no worldmap_filename
    // specified. Let's go ahead and use the worldmap
    // filename specified in the world.
    if (filename.empty())
    {
      filename = world.get_worldmap_filename();
    }

    auto worldmap = std::make_unique<worldmap::WorldMap>(filename, *m_savegame, sector, spawnpoint);
    auto worldmap_screen = std::make_unique<worldmap::WorldMapScreen>(std::move(worldmap));
    ScreenManager::current()->push_screen(std::move(worldmap_screen));

    if (!Editor::current())
      m_savegame->get_profile().set_last_world(world.get_basename());
  }
  catch(std::exception& e)
  {
    log_fatal << "Couldn't start world: " << e.what() << std::endl;
  }
}

void
GameManager::start_worldmap(const World& world, const std::string& worldmap_filename,
                            const std::optional<std::pair<std::string, Vector>>& start_pos)
{
  start_worldmap(world, worldmap_filename, start_pos ? start_pos->first : "");
  if (start_pos)
    worldmap::WorldMapSector::current()->get_tux().set_initial_pos(start_pos->second);
}

bool
GameManager::load_next_worldmap()
{
  if (!m_next_worldmap)
    return false;

  const auto next_worldmap = std::move(*m_next_worldmap);
  m_next_worldmap.reset();

  std::unique_ptr<World> world = World::from_directory(next_worldmap.world);
  if (!world)
  {
    log_warning << "Cannot load world '" << next_worldmap.world << "'" <<  std::endl;
    return false;
  }

  start_worldmap(*world, "", next_worldmap.sector, next_worldmap.spawnpoint); // New world, new savegame.
  return true;
}

void
GameManager::set_next_worldmap(const std::string& world, const std::string& sector,
                               const std::string& spawnpoint)
{
  m_next_worldmap.emplace(world, sector, spawnpoint);
}

void
GameManager::host_game(uint16_t port)
{
  if (m_network_server)
  {
    Dialog::show_message(_("A game is currently being hosted. Please try again later!"));
    return;
  }

  try
  {
    m_network_server = &network::HostManager::current()->create<network::Server>(port, 32);
  }
  catch (const std::exception& err)
  {
    log_warning << "Error starting network server: " << err.what() << std::endl;
    Dialog::show_message(_("Error starting network server:") + "\n\n" + err.what());
    return;
  }

  m_network_server->set_protocol(std::make_unique<GameNetworkProtocol>(*this, *m_network_server));
}

void
GameManager::connect_to_remote_game(const std::string& hostname, uint16_t port,
                                    const std::string& nickname, const Color& nickname_color)
{
  if (m_network_server_peer)
  {
    Dialog::show_message(_("A connection is currently active. Please try again later!"));
    return;
  }

  if (!GameNetworkProtocol::verify_nickname(nickname))
  {
    Dialog::show_message(_("The provided nickname is invalid. Please try again!"));
    return;
  }

  close_connections();

  try
  {
    m_network_client = &network::HostManager::current()->create<network::Client>(1);
  }
  catch (const std::exception& err)
  {
    log_warning << "Error starting network client: " << err.what() << std::endl;
    Dialog::show_message(_("Error starting network client:") + "\n" + err.what());
    return;
  }

  m_network_client->set_protocol(std::make_unique<GameNetworkProtocol>(*this, *m_network_client));

  auto connection = m_network_client->connect(hostname.c_str(), port, 1500);
  if (connection.status != network::ConnectionStatus::SUCCESS)
  {
    switch (connection.status)
    {
      case network::ConnectionStatus::FAILED_NO_PEERS:
        Dialog::show_message(_("Failed to connect: No peers available."));
        break;
      case network::ConnectionStatus::FAILED_TIMED_OUT:
        Dialog::show_message(_("Failed to connect: Connection request timed out."));
        break;
      case network::ConnectionStatus::FAILED_VERSION_MISMATCH:
        Dialog::show_message(_("Failed to connect: Local and server SuperTux versions do not match."));
        break;
      case network::ConnectionStatus::FAILED_CONNECTION_REFUSED:
        Dialog::show_message(_("Failed to connect: The server refused the connection."));
        break;
      default:
        Dialog::show_message(_("Failed to connect!"));
        break;
    }

    m_network_client->destroy();
    m_network_client = nullptr;
    m_network_server_peer = nullptr;
    return;
  }

  m_network_server_peer = connection.peer;

  // Request registration on the server.
  GameServerUser user(nickname, nickname_color, InputManager::current()->get_num_users());
  m_network_client->send_request(m_network_server_peer,
                                 std::make_unique<network::Request>(
                                   std::make_unique<network::StagedPacket>(GameNetworkProtocol::OP_USER_REGISTER,
                                     user.serialize(), 2.f),
                                   4.f));
}

void
GameManager::stop_hosting_game()
{
  // TODO: Leave active GameSession

  m_server_users.clear();

  m_network_server->destroy();
  m_network_server = nullptr;
}

void
GameManager::close_connections()
{
  m_server_users.clear();

  if (m_network_server_peer) // Client - disconnect from server
  {
    m_network_client->set_protocol({});
    m_network_client->disconnect(m_network_server_peer);
    m_network_client->destroy();
    m_network_client = nullptr;
    m_network_server_peer = nullptr;
  }
  else if (m_network_server) // Server - stop hosting
    stop_hosting_game();
}

/* EOF */
