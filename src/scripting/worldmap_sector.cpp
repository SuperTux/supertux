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

#include "scripting/worldmap_sector.hpp"

#include "worldmap/worldmap.hpp"

namespace scripting {

WorldMapSector::WorldMapSector(::worldmap::WorldMapSector* parent) :
  GameObjectManager(parent),
  m_parent(parent)
{
}

float
WorldMapSector::get_tux_x() const
{
  return m_parent->get_tux_pos().x;
}

float
WorldMapSector::get_tux_y() const
{
  return m_parent->get_tux_pos().y;
}

void
WorldMapSector::set_sector(const std::string& sector)
{
  SCRIPT_GUARD_WORLDMAP;
  worldmap.set_sector(sector);
}

void
WorldMapSector::spawn(const std::string& sector, const std::string& spawnpoint)
{
  SCRIPT_GUARD_WORLDMAP;
  worldmap.set_sector(sector, spawnpoint);
}

void
WorldMapSector::move_to_spawnpoint(const std::string& spawnpoint)
{
  m_parent->move_to_spawnpoint(spawnpoint);
}

} // namespace scripting

/* EOF */
