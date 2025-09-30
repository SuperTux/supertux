//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "supertux/sector_parser.hpp"

#include <iostream>
#include <physfs.h>
#include <sexp/value.hpp>

#include "badguy/fish_jumping.hpp"
#include "badguy/jumpy.hpp"
#include "editor/editor.hpp"
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
#include "supertux/constants.hpp"
#include "supertux/game_object_factory.hpp"
#include "supertux/level.hpp"
#include "supertux/sector.hpp"
#include "supertux/tile.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/spawn_point.hpp"

namespace
{
  static const std::string DEFAULT_BG = "images/background/antarctic/snow_hills.png";
  static const std::string DEFAULT_BG_TOP = "images/background/misc/transparent_up.png";
  static const std::string DEFAULT_BG_BOTTOM = "images/background/antarctic/snow_bottom.png";
}

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

SectorParser::SectorParser(Base::Sector& sector, bool editable) :
  m_sector(sector),
  m_editable(editable)
{
}

std::unique_ptr<GameObject>
SectorParser::parse_object(const std::string& name, const ReaderMapping& reader)
{
  if (parse_object_additional(name, reader))
    return {}; // Object was parsed by additional rules, so cancel regular object parsing.

  try
  {
    return GameObjectFactory::instance().create(name, reader);
  }
  catch (std::exception& err)
  {
    std::string obj_name;
    reader.get("name", obj_name);

    log_warning << "Couldn't parse object of type '" << name << "' with name '" << obj_name << "': " << err.what() << std::endl;
    return {};
  }
}

bool
SectorParser::parse_object_additional(const std::string& name, const ReaderMapping& reader)
{
  return false; // No additional object parsing rules, continue with regular object parsing.
}

void
SectorParser::parse(const ReaderMapping& reader)
{
  auto iter = reader.get_iter();
  while (iter.next()) {
    if (iter.get_key() == "name")
    {
      std::string value;
      iter.get(value);
      m_sector.set_name(value);
    }
    else if (iter.get_key() == "gravity")
    {
      auto sector = dynamic_cast<Sector*>(&m_sector);
      if (!sector) continue;

      float value;
      iter.get(value);
      sector->set_gravity(value);
    }
    else if (iter.get_key() == "music")
    {
      const auto& sx = iter.get_sexp();
      if (sx.is_array() && sx.as_array().size() == 2 && sx.as_array()[1].is_string()) {
        std::string value;
        iter.get(value);
        m_sector.add<MusicObject>().set_music(value);
      } else {
        m_sector.add<MusicObject>(iter.as_mapping());
      }
    }
    else if (iter.get_key() == "init-script")
    {
      std::string value;
      iter.get(value);
      m_sector.set_init_script(value);
    }
    else if(iter.get_key() == "init-script-run-once")
    {
      auto sector = dynamic_cast<Sector*>(&m_sector);
      if (!sector) continue;
      bool value;
      iter.get(value);
      sector->set_init_script_run_once(value);
    }
    else if (iter.get_key() == "ambient-light")
    {
      const auto& sx = iter.get_sexp();
      if (sx.is_array() && sx.as_array().size() >= 3 &&
          sx.as_array()[1].is_real() && sx.as_array()[2].is_real() && sx.as_array()[3].is_real())
      {
        // for backward compatibilty
        std::vector<float> vColor;
        bool hasColor = reader.get("ambient-light", vColor);
        if (vColor.size() < 3 || !hasColor) {
          log_warning << "(ambient-light) requires a color as argument" << std::endl;
        } else {
          m_sector.add<AmbientLight>(Color(vColor));
        }
      } else {
        // modern format
        m_sector.add<AmbientLight>(iter.as_mapping());
      }
    }
    else
    {
      auto object = parse_object(iter.get_key(), iter.as_mapping());
      if (object)
        m_sector.add_object(std::move(object));
    }
  }

  m_sector.finish_construction(m_editable);
}

void
SectorParser::parse_old_format(const ReaderMapping& reader)
{
  m_sector.set_name(DEFAULT_SECTOR_NAME);

  auto sector = dynamic_cast<Sector*>(&m_sector);
  if (sector)
  {
    float gravity;
    if (reader.get("gravity", gravity))
      sector->set_gravity(gravity);
  }

  std::string backgroundimage;
  if (reader.get("background", backgroundimage) && (!backgroundimage.empty())) {
    // These paths may need to be changed.
    //Delete 3 lines below once deprecated Backgrounds are deleted. 
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

  m_sector.add<SpawnPointMarker>(DEFAULT_SPAWNPOINT_NAME, startpos);

  auto& music = m_sector.add<MusicObject>();
  music.set_music("/music/antarctic/midday.music");

  int width = 30, height = 15;
  reader.get("width", width);
  reader.get("height", height);

  std::vector<unsigned int> tiles;
  if (reader.get("interactive-tm", tiles)
     || reader.get("tilemap", tiles)) {
    auto& tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
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
    auto& tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
    tilemap.set(width, height, tiles, LAYER_BACKGROUNDTILES, false);
    if (height < 19) tilemap.resize(width, 19);
  }

  if (reader.get("foreground-tm", tiles)) {
    auto& tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
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
        Vector sp_pos(0.0f, 0.0f);
        if (reader.get("x", sp_pos.x) && reader.get("y", sp_pos.y))
        {
          m_sector.add<SpawnPointMarker>(DEFAULT_SPAWNPOINT_NAME, sp_pos);
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
  if (!m_sector.in_worldmap())
  {
    auto& background = m_sector.add<Background>();
    background.set_images(DEFAULT_BG_TOP, DEFAULT_BG, DEFAULT_BG_BOTTOM);
    background.set_speed(0.3);
    background.set_name("Snowy_Hills");

    auto& panorama = m_sector.add<Background>();
    panorama.set_image("images/background/antarctic/snow_panorama.png");
    panorama.set_images(DEFAULT_BG_TOP, "images/background/antarctic/snow_panorama.png", "images/background/misc/water_bottom.png");
    panorama.set_speed(0.0);
    panorama.set_layer(-350);
    panorama.set_name("Panorama");

    auto& midground_tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
    midground_tilemap.resize(Sector::DEFAULT_SECTOR_WIDTH, Sector::DEFAULT_SECTOR_HEIGHT);
    midground_tilemap.set_layer(-100);
    midground_tilemap.set_solid(false);
    midground_tilemap.set_name("MidGround_Tiles");

    auto& background_tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
    background_tilemap.resize(Sector::DEFAULT_SECTOR_WIDTH, Sector::DEFAULT_SECTOR_HEIGHT);
    background_tilemap.set_layer(-150);
    background_tilemap.set_solid(false);
    background_tilemap.set_name("BackGround_Tiles");

    auto& gradient = m_sector.add<Gradient>();
    gradient.set_gradient(Gradient::DEFAULT_GRADIENT_TOP, Gradient::DEFAULT_GRADIENT_BOTTOM);
    gradient.set_layer(-400);
  }
  else
  {
    auto& water = m_sector.add<TileMap>(m_sector.get_tileset());
    water.resize(DEFAULT_WORLDMAP_WIDTH, DEFAULT_WORLDMAP_HEIGHT, 1);
    water.set_layer(-100);
    water.set_solid(false);
    water.set_name("Water_Tiles");
  }

  auto& music = m_sector.add<MusicObject>();
  music.set_name("Music_Object");

  auto& main_tilemap = m_sector.add<TileMap>(m_sector.get_tileset());
  main_tilemap.set_layer(0);
  main_tilemap.set_solid(true);
  main_tilemap.set_name("Main_Tiles");

  if (m_sector.in_worldmap())
  {
    main_tilemap.resize(DEFAULT_WORLDMAP_WIDTH, DEFAULT_WORLDMAP_HEIGHT, 0);

    m_sector.add<worldmap::SpawnPointObject>(DEFAULT_SPAWNPOINT_NAME, Vector(4, 4));
  }
  else
  {
    main_tilemap.resize(Sector::DEFAULT_SECTOR_WIDTH, Sector::DEFAULT_SECTOR_HEIGHT, 0);

    m_sector.add<SpawnPointMarker>(DEFAULT_SPAWNPOINT_NAME, Vector(64, 480));

    music.set_music("/music/antarctic/midday.music");
  }

  m_sector.add<Camera>("Camera");

  m_sector.finish_construction(m_editable);
}
