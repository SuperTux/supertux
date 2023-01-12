//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
//                2023 Vankata453
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

#ifndef HEADER_SUPERTUX_SCRIPTING_WORLDMAP_SECTOR_HPP
#define HEADER_SUPERTUX_SCRIPTING_WORLDMAP_SECTOR_HPP

#ifndef SCRIPTING_API

#include <string>

#include "scripting/game_object_manager.hpp"

namespace worldmap {
  class WorldMapSector;
}

/** Macro to allow quick and easy access to the parent worldmap. **/
#define SCRIPT_GUARD_WORLDMAP                                               \
  auto& worldmap = m_parent->get_worldmap()

#endif

namespace scripting {

class WorldMapSector final : public GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::worldmap::WorldMapSector* m_parent;

public:
  WorldMapSector(::worldmap::WorldMapSector* parent);

private:
  WorldMapSector(const WorldMapSector&) = delete;
  WorldMapSector& operator=(const WorldMapSector&) = delete;
#endif

public:
  float get_tux_x();
  float get_tux_y();

  /**
   * Changes the current sector of the worldmap to a specified new sector.
   */
  void set_sector(const std::string& sector);

  /**
   * Changes the current sector of the worldmap to a specified new sector,
   * as well as moves Tux to a specified spawnpoint.
   */
  void spawn(const std::string& sector, const std::string& spawnpoint);

  /**
   * Moves Tux to a specified spawnpoint.
   */
  void move_to_spawnpoint(const std::string& spawnpoint);
};

} // namespace scripting

#endif

/* EOF */
