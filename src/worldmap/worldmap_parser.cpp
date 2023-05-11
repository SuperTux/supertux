//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "worldmap/worldmap_parser.hpp"

#include "physfs/util.hpp"
#include "supertux/d_scope.hpp"
#include "util/file_system.hpp"
#include "worldmap/spawn_point.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

const std::string WorldMapParser::s_default_tileset = "images/ice_world.strf";

WorldMapParser::WorldMapParser(WorldMap& worldmap) :
  LevelParser(worldmap, true, false),
  m_worldmap(worldmap)
{
}

void
WorldMapParser::load(const std::string& filepath)
{
  m_worldmap.m_map_filename = physfsutil::realpath(filepath);
  m_worldmap.m_levels_path = FileSystem::dirname(m_worldmap.m_map_filename);

  LevelParser::load(filepath);
}

void
WorldMapParser::add_sector(const ReaderMapping& reader)
{
  m_worldmap.add_sector(WorldMapSectorParser::from_reader(m_worldmap, reader));
}



std::unique_ptr<WorldMapSector>
WorldMapSectorParser::from_reader(WorldMap& worldmap, const ReaderMapping& reader)
{
  auto sector = std::make_unique<WorldMapSector>(worldmap);
  BIND_WORLDMAP_SECTOR(*sector);
  WorldMapSectorParser parser(*sector);
  parser.parse(reader);
  return sector;
}


WorldMapSectorParser::WorldMapSectorParser(WorldMapSector& sector) :
  SectorParser(sector, false),
  m_worldmap_sector(sector)
{
}

bool
WorldMapSectorParser::parse_object_additional(const std::string& name, const ReaderMapping& reader)
{
  if (name == "worldmap-spawnpoint") // Custom rule for adding spawnpoints
  {
    m_worldmap_sector.m_spawnpoints.push_back(std::make_unique<SpawnPoint>(reader));
    return true;
  }
  return false;
}

} // namespace worldmap

/* EOF */
