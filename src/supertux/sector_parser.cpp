//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/sector_parser.hpp"

#include <iostream>
#include <physfs.h>
#include <sexp/value.hpp>

#include "badguy/jumpy.hpp"
#include "editor/editor.hpp"
#include "editor/worldmap_objects.hpp"
#include "object/ambient_light.hpp"
#include "object/background.hpp"
#include "object/camera.hpp"
#include "object/cloud_particle_system.hpp"
#include "object/custom_particle_system.hpp"
#include "object/gradient.hpp"
#include "object/music_object.hpp"
#include "object/rain_particle_system.hpp"
#include "object/snow_particle_system.hpp"
#include "object/spawnpoint.hpp"
#include "object/tilemap.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"

static const std::string DEFAULT_BG = "images/background/antarctic/arctis2.png";

std::unique_ptr<Sector>
SectorParser::from_reader(Level& level, const ReaderMapping& reader, bool editable)
{
  auto sector = std::make_unique<Sector>(level);
  BIND_SECTOR(*sector);
  SectorParser parser(*sector, editable);
  parser.parse(reader);
  return sector;
}

std::unique_ptr<Sector>
SectorParser::from_reader_old_format(Level& level, const ReaderMapping& reader, bool editable)
{
  auto sector = std::make_unique<Sector>(level);
  BIND_SECTOR(*sector);
  SectorParser parser(*sector, editable);
  parser.parse_old_format(reader);
  return sector;
}

std::unique_ptr<Sector>
SectorParser::from_nothing(Level& level)
{
  auto sector = std::make_unique<Sector>(level);
  BIND_SECTOR(*sector);
  SectorParser parser(*sector, false);
  parser.create_sector();
  return sector;
}

SectorParser::SectorParser(Sector& sector, bool editable) :
  m_sector(sector),
  m_editable(editable)
{
}

std::unique_ptr<GameObject>
SectorParser::parse_object(const std::string& name_, const ReaderMapping& reader)
{
  if (name_ == "money") { // for compatibility with old maps
    return std::make_unique<Jumpy>(reader);
  } else {
    try {
      return GameObjectFactory::instance().create(name_, reader);
    } catch(std::exception& e) {
      log_warning << e.what() << "" << std::endl;
      return {};
    }
  }
}

void
SectorParser::parse(const ReaderMapping& sector)
{
  auto iter = sector.get_iter();
  while (iter.next()) {
    if (iter.get_key() == "name") {
      std::string value;
      iter.get(value);
      m_sector.set_name(value);
    } else if (iter.get_key() == "gravity") {
      float value;
      iter.get(value);
      m_sector.set_gravity(value);
    } else if (iter.get_key() == "music") {
      const auto& sx = iter.get_sexp();
      if (sx.is_array() && sx.as_array().size() == 2 && sx.as_array()[1].is_string()) {
        std::string value;
        iter.get(value);
        m_sector.add<MusicObject>().set_music(value);
      } else {
        m_sector.add<MusicObject>(iter.as_mapping());
      }
    } else if (iter.get_key() == "init-script") {
      std::string value;
      iter.get(value);
      m_sector.set_init_script(value);
    } else if (iter.get_key() == "ambient-light") {
      const auto& sx = iter.get_sexp();
      if (sx.is_array() && sx.as_array().size() >= 3 &&
          sx.as_array()[1].is_real() && sx.as_array()[2].is_real() && sx.as_array()[3].is_real())
      {
        // for backward compatibilty
        std::vector<float> vColor;
        bool hasColor = sector.get("ambient-light", vColor);
        if (vColor.size() < 3 || !hasColor) {
          log_warning << "(ambient-light) requires a color as argument" << std::endl;
        } else {
          m_sector.add<AmbientLight>(Color(vColor));
        }
      } else {
        // modern format
        m_sector.add<AmbientLight>(iter.as_mapping());
      }
    } else {
      auto object = parse_object(iter.get_key(), iter.as_mapping());
      if (object) {
        m_sector.add_object(std::move(object));
      }
    }
  }

  m_sector.finish_construction(m_editable);
}

void
SectorParser::parse_old_format(const ReaderMapping& reader)
{
  m_sector.set_name("main");

  float gravity;
  if (reader.get("gravity", gravity))
    m_sector.set_gravity(gravity);

  std::string backgroundimage;
  if (reader.get("background", backgroundimage) && (!backgroundimage.empty())) {
    // These paths may need to be changed.
    if (backgroundimage == "arctis.png") backgroundimage = "arctis.jpg";
    if (backgroundimage == "arctis2.jpg") backgroundimage = "arctis.jpg";
    if (backgroundimage == "ocean.png") backgroundimage = "ocean.jpg";
    backgroundimage = "images/background/" + backgroundimage;
    if (!PHYSFS_exists(backgroundimage.c_str())) {
      log_warning << "Background image \"" << backgroundimage << "\" not found. Ignoring." << std::endl;
      backgroundimage = "";
    }
  }

  float bgspeed = .5;
  reader.get("bkgd_speed", bgspeed);
  bgspeed /= 100;

  Color bkgd_top, bkgd_bottom;
  int r = 0, g = 0, b = 128;
  reader.get("bkgd_red_top", r);
  reader.get("bkgd_green_top",  g);
  reader.get("bkgd_blue_top",  b);
  bkgd_top.red = static_cast<float> (r) / 255.0f;
  bkgd_top.green = static_cast<float> (g) / 255.0f;
  bkgd_top.blue = static_cast<float> (b) / 255.0f;

  reader.get("bkgd_red_bottom",  r);
  reader.get("bkgd_green_bottom", g);
  reader.get("bkgd_blue_bottom", b);
  bkgd_bottom.red = static_cast<float> (r) / 255.0f;
  bkgd_bottom.green = static_cast<float> (g) / 255.0f;
  bkgd_bottom.blue = static_cast<float> (b) / 255.0f;

  if (!backgroundimage.empty()) {
    auto& background = m_sector.add<Background>();
    background.set_image(backgroundimage);
    background.set_speed(bgspeed);
  } else {
    auto& gradient = m_sector.add<Gradient>();
    gradient.set_gradient(bkgd_top, bkgd_bottom);
  }

  std::string particlesystem;
  reader.get("particle_system", particlesystem);
  if (particlesystem == "clouds")
    m_sector.add<CloudParticleSystem>();
  else if (particlesystem == "snow")
    m_sector.add<SnowParticleSystem>();
  else if (particlesystem == "rain")
    m_sector.add<RainParticleSystem>();
  else if (particlesystem == "custom-particles")
    m_sector.add<CustomParticleSystem>();

  Vector startpos(100, 170);
  reader.get("start_pos_x", startpos.x);
  reader.get("start_pos_y", startpos.y);

  m_sector.add<SpawnPointMarker>("main", startpos);

  m_sector.add<MusicObject>().set_music("music/chipdisko.ogg");
  // skip reading music filename. It's all .ogg now, anyway
  /*
    reader.get("music", music);
    m_sector.set_music("music/" + m_sector.get_music());
  */

  int width = 30, height = 15;
  reader.get("width", width);
  reader.get("height", height);

  std::vector<unsigned int> tiles;
  if (reader.get("interactive-tm", tiles)
     || reader.get("tilemap", tiles)) {
    auto* tileset = TileManager::current()->get_tileset(m_sector.get_level().get_tileset());
    auto& tilemap = m_sector.add<TileMap>(tileset);
    tilemap.set(width, height, tiles, LAYER_TILES, true);

    // replace tile id 112 (old invisible tile) with 1311 (new invisible tile)
    for (int x=0; x < tilemap.get_width(); ++x) {
      for (int y=0; y < tilemap.get_height(); ++y) {
        uint32_t id = tilemap.get_tile_id(x, y);
        if (id == 112)
          tilemap.change(x, y, 1311);
      }
    }

    if (height < 19) tilemap.resize(width, 19);
  }

  if (reader.get("background-tm", tiles)) {
    auto* tileset = TileManager::current()->get_tileset(m_sector.get_level().get_tileset());
    auto& tilemap = m_sector.add<TileMap>(tileset);
    tilemap.set(width, height, tiles, LAYER_BACKGROUNDTILES, false);
    if (height < 19) tilemap.resize(width, 19);
  }

  if (reader.get("foreground-tm", tiles)) {
    auto* tileset = TileManager::current()->get_tileset(m_sector.get_level().get_tileset());
    auto& tilemap = m_sector.add<TileMap>(tileset);
    tilemap.set(width, height, tiles, LAYER_FOREGROUNDTILES, false);

    // fill additional space in foreground with tiles of ID 2035 (lightmap/black)
    if (height < 19) tilemap.resize(width, 19, 2035);
  }

  // read reset-points (now spawn-points)
  std::optional<ReaderMapping> resetpoints;
  if (reader.get("reset-points", resetpoints)) {
    auto iter = resetpoints->get_iter();
    while (iter.next()) {
      if (iter.get_key() == "point") {
        Vector sp_pos;
        if (reader.get("x", sp_pos.x) && reader.get("y", sp_pos.y))
        {
          m_sector.add<SpawnPointMarker>("main", sp_pos);
        }
      } else {
        log_warning << "Unknown token '" << iter.get_key() << "' in reset-points." << std::endl;
      }
    }
  }

  // read objects
  std::optional<ReaderCollection> objects;
  if (reader.get("objects", objects)) {
    for (auto const& obj : objects->get_objects())
    {
      auto object = parse_object(obj.get_name(), obj.get_mapping());
      if (object) {
        m_sector.add_object(std::move(object));
      } else {
        log_warning << "Unknown object '" << obj.get_name() << "' in level." << std::endl;
      }
    }
  }

  // add a camera
  auto camera_ = std::make_unique<Camera>("Camera");
  m_sector.add_object(std::move(camera_));

  m_sector.flush_game_objects();

  if (m_sector.get_solid_tilemaps().empty()) {
    log_warning << "sector '" << m_sector.get_name() << "' does not contain a solid tile layer." << std::endl;
  }

  m_sector.finish_construction(m_editable);
}

void
SectorParser::create_sector()
{
  auto tileset = TileManager::current()->get_tileset(m_sector.get_level().get_tileset());
  bool worldmap = m_sector.get_level().is_worldmap();
  if (!worldmap)
  {
    auto& background = m_sector.add<Background>();
    background.set_image(DEFAULT_BG);
    background.set_speed(0.5);

    auto& bkgrd = m_sector.add<TileMap>(tileset);
    bkgrd.resize(100, 35);
    bkgrd.set_layer(-100);
    bkgrd.set_solid(false);

    auto& frgrd = m_sector.add<TileMap>(tileset);
    frgrd.resize(100, 35);
    frgrd.set_layer(100);
    frgrd.set_solid(false);
  }

  auto& intact = m_sector.add<TileMap>(tileset);
  if (worldmap) {
    intact.resize(100, 100, 9);
  } else {
    intact.resize(100, 35, 0);
  }
  intact.set_layer(0);
  intact.set_solid(true);

  if (worldmap) {
    m_sector.add<worldmap_editor::WorldmapSpawnPoint>("main", Vector(4, 4));
  } else {
    m_sector.add<SpawnPointMarker>("main", Vector(64, 480));
  }

  m_sector.add<Camera>("Camera");
  m_sector.add<MusicObject>();

  m_sector.flush_game_objects();

  m_sector.finish_construction(m_editable);
}

/* EOF */
