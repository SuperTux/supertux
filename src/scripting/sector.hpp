//  SuperTux - Sector scripting
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
//                2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_SECTOR_HPP
#define HEADER_SUPERTUX_SCRIPTING_SECTOR_HPP

#ifndef SCRIPTING_API
#include <string>
#include "scripting/game_object_manager.hpp"
class Sector;
#endif

namespace scripting {

/**
 * @summary This class provides additional controlling functions for a sector, other than the ones listed at ${SRG_REF_GameObjectManager}.
 * @instances An instance under ""sector.settings"" is available from scripts and the console.
 */
class Sector final : public GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::Sector* m_parent;

public:
  Sector(::Sector* parent);

private:
  Sector(const Sector&) = delete;
  Sector& operator=(const Sector&) = delete;
#endif

public:
  /**
   * Sets the sector's gravity.
   * @param float $gravity
   */
  void set_gravity(float gravity);

  /**
   * Adds a MovingObject to the game.
   * 
   * @param string $class_name GameObject's class.
   * @param string $name Name of the created object.
   * @param int $posX X position inside the current sector.
   * @param int $posY Y position inside the current sector.
   * @param string $direction Direction.
   * @param string $data Additional data.
   */
void add_object(const std::string& class_name, const std::string& name,
                int posX, int posY, const std::string& direction,
                const std::string& data);
};

} // namespace scripting

#endif

/* EOF */
