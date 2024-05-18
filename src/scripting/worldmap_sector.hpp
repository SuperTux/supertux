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
#define SCRIPT_GUARD_WORLDMAP               \
  auto& worldmap = m_parent->get_worldmap()

#endif

namespace scripting {

/**
 * @summary This class provides additional controlling functions for a worldmap sector, other than the ones listed at ${SRG_REF_GameObjectManager}.
 * @instances An instance under ""worldmap.settings"" is available from scripts and the console.
 */
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
  /**
   * Gets Tux's X position on the worldmap.
   */
  float get_tux_x() const;
  /**
   * Gets Tux's Y position on the worldmap.
   */
  float get_tux_y() const;

  /**
   * Changes the current sector of the worldmap to a specified new sector.
   * @param string $sector
   */
  void set_sector(const std::string& sector);

  /**
   * Changes the current sector of the worldmap to a specified new sector,
     moving Tux to the specified spawnpoint.
   * @param string $sector
   * @param string $spawnpoint
   */
  void spawn(const std::string& sector, const std::string& spawnpoint);

  /**
   * Moves Tux to a specified spawnpoint.
   * @param string $spawnpoint
   */
  void move_to_spawnpoint(const std::string& spawnpoint);

  /**
   * Gets the path to the worldmap file. Useful for saving worldmap specific data.
   */
  std::string get_filename() const;

  /**
   * Overrides the "Title Screen Level" property for the world with ""filename"".
     The newly set level will be used for the title screen, after exiting the world.
   * @param string $filename
   */
  void set_title_level(const std::string& filename);
};

} // namespace scripting

#endif

/* EOF */
