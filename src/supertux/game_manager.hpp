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

#include <memory>
#include <string>
#include "util/currenton.hpp"

class Savegame;
class World;

class GameManager : public Currenton<GameManager>
{
private:
  std::unique_ptr<World> m_world;
  std::unique_ptr<Savegame> m_savegame;

  void run_level(World* world, const std::string& level_filename);
  void run_worldmap(World* world, const std::string& worldmap_filename, const std::string& spawnpoint);

public:
  GameManager();
  ~GameManager();

  void start_worldmap(std::unique_ptr<World> world, const std::string& spawnpoint = "", const std::string& worldmap_filename = "");
  void start_worldmap(World* world, const std::string& spawnpoint = "", const std::string& worldmap_filename = "");

  void start_level(std::unique_ptr<World> world, const std::string& level_filename);
  /**
   * This method is to be called when we don't want to give up ownership of the
   * world unique_ptr. This is specifically the case for when levels are started
   * from the editor.
   */
  void start_level(World* world, const std::string& level_filename);

  std::string get_level_name(const std::string& levelfile) const;

  bool load_next_worldmap();
  void set_next_worldmap(const std::string& worldmap, const std::string &spawnpoint);

private:
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  std::string next_worldmap;
  std::string next_spawnpoint;
};

#endif

/* EOF */
