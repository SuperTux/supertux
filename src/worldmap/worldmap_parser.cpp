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
#include "supertux/tile_manager.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "worldmap/level_tile.hpp"
#include "worldmap/spawn_point.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/teleporter.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

/* WorldMap parser */

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

/* WorldMapSector parser */

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
  // Custom rules for adding worldmap objects.
  if (name == "level")
  {
    auto& level_tile = m_worldmap_sector.add<LevelTile>(m_worldmap_sector.get_worldmap().get_levels_path(), reader);
    load_level_information(level_tile);
    return true;
  }
  else if (name == "special-tile")
  {
    m_worldmap_sector.add<SpecialTile>(reader);
    return true;
  }
  else if (name == "sprite-change")
  {
    m_worldmap_sector.add<SpriteChange>(reader);
    return true;
  }
  else if (name == "teleporter")
  {
    m_worldmap_sector.add<Teleporter>(reader);
    return true;
  }
  else if (name == "worldmap-spawnpoint")
  {
    m_worldmap_sector.m_spawn_points.push_back(std::make_unique<SpawnPoint>(reader));
    return true;
  }

  return false;
}


void
WorldMapSectorParser::load_level_information(LevelTile& level_tile)
{
  /** Set default LevelTile properties. */
  level_tile.m_title = _("<no title>");
  level_tile.m_target_time = 0.0f;

  try
  {
    // Determine the level filename.
    const std::string& levels_path = m_worldmap_sector.get_worldmap().get_levels_path();
    std::string filename = levels_path + level_tile.get_level_filename();

    if (levels_path == "./")
      filename = level_tile.get_level_filename();

    try
    {
      // Read general level info.
      auto level = LevelParser::from_file(filename, false, false, true, true);

      // Set the LevelTile's properties to the ones of the level.
      level_tile.m_title = level->m_name;
      level_tile.m_target_time = level->m_target_time;
    }
    catch (std::exception& err)
    {
      std::stringstream out;
      out << "Cannot read level info: " << err.what() << std::endl;
      throw std::runtime_error(out.str());
    }
  }
  catch (std::exception& err)
  {
    log_warning << "Problem when reading level information: " << err.what() << std::endl;
    return;
  }
}

} // namespace worldmap

/* EOF */
