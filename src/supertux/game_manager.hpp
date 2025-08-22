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

#pragma once

#include "util/currenton.hpp"

#include <memory>
#include <optional>
#include <string>
#include <sstream>

#include "math/vector.hpp"

class Savegame;
class World;
class Level;
namespace worldmap {
  class WorldMap;
}

class GameManager final : public Currenton<GameManager>
{
public:
  GameManager();
  
  void save();

  worldmap::WorldMap* create_worldmap_instance(const World& world, const std::string& worldmap_filename = "",
                                               const std::string& sector = "", const std::string& spawnpoint = "");
  bool start_worldmap(const World& world, const std::string& worldmap_filename = "",
                      const std::string& sector = "", const std::string& spawnpoint = "");
  bool start_worldmap(const World& world, const std::string& worldmap_filename,
                      const std::optional<std::pair<std::string, Vector>>& start_pos);
  void start_level(const World& world, const std::string& level_filename,
                   const std::optional<std::pair<std::string, Vector>>& start_pos = std::nullopt,
                   bool skip_intro = false);
  void start_level(Level* level, const std::optional<std::pair<std::string, Vector>>& start_pos = std::nullopt,
                   bool skip_intro = false);

public:
  std::unique_ptr<Savegame> m_savegame;
  
private:
  std::unique_ptr<Level>    m_current_level;
  
  // Must keep stringstream in memory or else GameSession can't restart.
  std::stringstream m_levelstream;

private:
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;
};
