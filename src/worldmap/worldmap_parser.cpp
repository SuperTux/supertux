//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include <physfs.h>
#include <sstream>

#include "object/ambient_light.hpp"
#include "object/background.hpp"
#include "object/decal.hpp"
#include "object/music_object.hpp"
#include "object/path_gameobject.hpp"
#include "object/tilemap.hpp"
#include "physfs/physfs_file_system.hpp"
#include "physfs/util.hpp"
#include "supertux/tile_manager.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_document.hpp"
#include "util/reader_mapping.hpp"
#include "util/reader_object.hpp"
#include "worldmap/level_tile.hpp"
#include "worldmap/spawn_point.hpp"
#include "worldmap/special_tile.hpp"
#include "worldmap/sprite_change.hpp"
#include "worldmap/teleporter.hpp"
#include "worldmap/tux.hpp"
#include "worldmap/worldmap.hpp"
#include "worldmap/worldmap_parser.hpp"
#include "worldmap/worldmap_screen.hpp"

namespace worldmap {

WorldMapParser::WorldMapParser(WorldMap& worldmap) :
  m_worldmap(worldmap)
{
}

void
WorldMapParser::load_worldmap(const std::string& filename)
{
  m_worldmap.m_map_filename = physfsutil::realpath(filename);
  m_worldmap.m_levels_path = FileSystem::dirname(m_worldmap.m_map_filename);

  try {
    register_translation_directory(m_worldmap.m_map_filename);
    auto doc = ReaderDocument::from_file(m_worldmap.m_map_filename);
    auto root = doc.get_root();

    if (root.get_name() != "supertux-level")
      throw std::runtime_error("file isn't a supertux-level file.");

    auto level_ = root.get_mapping();

    level_.get("name", m_worldmap.m_name);

    std::string tileset_name;
    if (level_.get("tileset", tileset_name)) {
      if (m_worldmap.m_tileset != nullptr) {
        log_warning << "multiple tilesets specified in level_" << std::endl;
      } else {
        m_worldmap.m_tileset = TileManager::current()->get_tileset(tileset_name);
      }
    }
    /* load default tileset */
    if (m_worldmap.m_tileset == nullptr) {
      m_worldmap.m_tileset = TileManager::current()->get_tileset("images/ice_world.strf");
    }

    std::optional<ReaderMapping> sector;
    if (!level_.get("sector", sector)) {
      throw std::runtime_error("No sector specified in worldmap file.");
    } else {
      auto iter = sector->get_iter();
      while (iter.next()) {
        if (iter.get_key() == "tilemap") {
          m_worldmap.add<TileMap>(m_worldmap.m_tileset, iter.as_mapping());
        } else if (iter.get_key() == "background") {
          m_worldmap.add<Background>(iter.as_mapping());
        } else if (iter.get_key() == "music") {
          const auto& sx = iter.get_sexp();
          if (sx.is_array() && sx.as_array().size() == 2 && sx.as_array()[1].is_string()) {
            std::string value;
            iter.get(value);
            m_worldmap.add<MusicObject>().set_music(value);
          } else {
            m_worldmap.add<MusicObject>(iter.as_mapping());
          }
        } else if (iter.get_key() == "init-script") {
          iter.get(m_worldmap.m_init_script);
        } else if (iter.get_key() == "worldmap-spawnpoint") {
          auto sp = std::make_unique<SpawnPoint>(iter.as_mapping());
          m_worldmap.m_spawn_points.push_back(std::move(sp));
        } else if (iter.get_key() == "level") {
          auto& level = m_worldmap.add<LevelTile>(m_worldmap.m_levels_path, iter.as_mapping());
          load_level_information(level);
        } else if (iter.get_key() == "special-tile") {
          m_worldmap.add<SpecialTile>(iter.as_mapping());
        } else if (iter.get_key() == "sprite-change") {
          m_worldmap.add<SpriteChange>(iter.as_mapping());
        } else if (iter.get_key() == "teleporter") {
          m_worldmap.add<Teleporter>(iter.as_mapping());
        } else if (iter.get_key() == "decal") {
          m_worldmap.add<Decal>(iter.as_mapping());
        } else if (iter.get_key() == "path") {
          m_worldmap.add<PathGameObject>(iter.as_mapping());
        } else if (iter.get_key() == "ambient-light") {
          const auto& sx = iter.get_sexp();
          if (sx.is_array() && sx.as_array().size() >= 3 &&
              sx.as_array()[1].is_real() && sx.as_array()[2].is_real() && sx.as_array()[3].is_real())
          {
            // for backward compatibilty
            std::vector<float> vColor;
            bool hasColor = sector->get("ambient-light", vColor);
            if (vColor.size() < 3 || !hasColor) {
              log_warning << "(ambient-light) requires a color as argument" << std::endl;
            } else {
              m_worldmap.add<AmbientLight>(Color(vColor));
            }
          } else {
            // modern format
            m_worldmap.add<AmbientLight>(iter.as_mapping());
          }
        } else if (iter.get_key() == "name") {
          // skip
        } else {
          log_warning << "Unknown token '" << iter.get_key() << "' in worldmap" << std::endl;
        }
      }
    }

    m_worldmap.flush_game_objects();

    if (m_worldmap.get_solid_tilemaps().empty())
      log_warning << "No solid tilemap specified" << std::endl;

    m_worldmap.move_to_spawnpoint("main");

  } catch(std::exception& e) {
    std::stringstream msg;
    msg << "Problem when parsing worldmap '" << m_worldmap.m_map_filename << "': " <<
      e.what();
    throw std::runtime_error(msg.str());
  }

  m_worldmap.finish_construction();
}

void
WorldMapParser::load_level_information(LevelTile& level)
{
  /** get special_tile's title */
  level.m_title = _("<no title>");
  level.m_target_time = 0.0f;

  try {
    std::string filename = m_worldmap.m_levels_path + level.get_level_filename();

    if (m_worldmap.m_levels_path == "./")
      filename = level.get_level_filename();

    if (!PHYSFS_exists(filename.c_str()))
    {
      log_warning << "Level file '" << filename << "' does not exist. Skipping." << std::endl;
      return;
    }
    if (physfsutil::is_directory(filename))
    {
      log_warning << "Level file '" << filename << "' is a directory. Skipping." << std::endl;
      return;
    }

    register_translation_directory(filename);
    auto doc = ReaderDocument::from_file(filename);
    auto root = doc.get_root();
    if (root.get_name() != "supertux-level") {
      return;
    } else {
      auto level_mapping = root.get_mapping();
      level_mapping.get("name", level.m_title);
      level_mapping.get("target-time", level.m_target_time);
    }
  } catch(std::exception& e) {
    log_warning << "Problem when reading level information: " << e.what() << std::endl;
    return;
  }
}

} // namespace worldmap

/* EOF */
