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
   * Checks if the specified sector-relative rectangle is free of solid tiles.
   * @param float $left
   * @param float $top
   * @param float $right
   * @param float $bottom
   * @param bool $ignore_unisolid If ""true"", unisolid tiles will be ignored.
   */
  bool is_free_of_solid_tiles(float left, float top, float right, float bottom,
                              bool ignore_unisolid) const;
  /**
   * Checks if the specified sector-relative rectangle is free of both:
       1) Solid tiles.
       2) ""MovingObject""s in ""COLGROUP_STATIC"".
     Note that this does not include badguys or players.
   * @param float $left
   * @param float $top
   * @param float $right
   * @param float $bottom
   * @param bool $ignore_unisolid If ""true"", unisolid tiles will be ignored.
   */
  bool is_free_of_statics(float left, float top, float right, float bottom,
                          bool ignore_unisolid) const;
  /**
   * Checks if the specified sector-relative rectangle is free of both:
       1) Solid tiles.
       2) ""MovingObject""s in ""COLGROUP_STATIC"", ""COLGROUP_MOVINGSTATIC"" or ""COLGROUP_MOVING"".
     This includes badguys and players.
   * @param float $left
   * @param float $top
   * @param float $right
   * @param float $bottom
   */
  bool is_free_of_movingstatics(float left, float top, float right, float bottom) const;
  /**
   * Checks if the specified sector-relative rectangle is free of ""MovingObject""s in ""COLGROUP_MOVINGSTATIC"".
     Note that this does not include moving badguys or players.
   * @param float $left
   * @param float $top
   * @param float $right
   * @param float $bottom
   */
  bool is_free_of_specifically_movingstatics(float left, float top, float right, float bottom) const;
};

} // namespace scripting

#endif

/* EOF */
