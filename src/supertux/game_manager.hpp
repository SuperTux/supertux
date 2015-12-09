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

#include "supertux/savegame.hpp"
#include "util/currenton.hpp"

class World;

class GameManager : public Currenton<GameManager>
{
private:
  std::unique_ptr<World> m_world;
  std::unique_ptr<Savegame> m_savegame;

public:
  GameManager();
  ~GameManager();

  void start_worldmap(std::unique_ptr<World> world);
  void start_level(std::unique_ptr<World> world, const std::string& level_filename);

  std::string get_level_name(const std::string& levelfile) const;

private:
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;
};

#endif

/* EOF */
