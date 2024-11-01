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

#ifndef HEADER_SUPERTUX_SUPERTUX_GAME_MANAGER_HPP
#define HEADER_SUPERTUX_SUPERTUX_GAME_MANAGER_HPP

#include "network/user_manager.hpp"
#include "util/currenton.hpp"

#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "math/vector.hpp"
#include "supertux/game_network_server_user.hpp"

class Color;
class GameSession;
class Savegame;
class Statistics;
class World;

namespace network {
class Client;
class Server;
} // namespace network

typedef struct _ENetPeer ENetPeer;

class GameManager final : public Currenton<GameManager>,
                          public network::UserManager<GameServerUser>
{
  friend class GameNetworkProtocol;

public:
  GameManager();

  void start_worldmap(const World& world, const std::string& worldmap_filename = "",
                      const std::string& sector = "", const std::string& spawnpoint = "");
  void start_worldmap(const World& world, const std::string& worldmap_filename,
                      const std::optional<std::pair<std::string, Vector>>& start_pos);
  void start_level(const World& world, const std::string& level_filename,
                   const std::optional<std::pair<std::string, Vector>>& start_pos = std::nullopt);
  void start_worldmap_level(const std::string& level_filename, Savegame& savegame,
                            Statistics* statistics);

  bool load_next_worldmap();
  void set_next_worldmap(const std::string& world, const std::string& sector = "",
                         const std::string& spawnpoint = "");

  void host_game(uint16_t port, const std::string& username, const Color& username_color);
  void stop_hosting_game();

  void connect_to_remote_game(const std::string& hostname, uint16_t port,
                              const std::string& username, const Color& username_color);

  ENetPeer* get_server_peer() const { return m_network_server_peer; }

private:
  GameSession* start_network_level(const std::string& remote_username, const std::string& player_status,
                                   const std::string& level_content);

  /** If server, stop hosting.
      If client, disconnect from server. */
  void close_connections();

private:
  struct NextWorldMap
  {
    NextWorldMap(const std::string& w, const std::string& s,
                 const std::string& sp) :
      world(w), sector(s), spawnpoint(sp)
    {}

    const std::string world;
    const std::string sector;
    const std::string spawnpoint;
  };

private:
  std::unique_ptr<Savegame> m_savegame;

  std::optional<NextWorldMap> m_next_worldmap;

  network::Server* m_network_server;
  network::Client* m_network_client;
  ENetPeer* m_network_server_peer;

private:
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;
};

#endif

/* EOF */
