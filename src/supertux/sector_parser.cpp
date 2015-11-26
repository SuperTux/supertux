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

#include "supertux/sector.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/jumpy.hpp"
#include "math/aatriangle.hpp"
#include "object/background.hpp"
#include "object/bonus_block.hpp"
#include "object/brick.hpp"
#include "object/bullet.hpp"
#include "object/camera.hpp"
#include "object/cloud_particle_system.hpp"
#include "object/coin.hpp"
#include "object/comet_particle_system.hpp"
#include "object/display_effect.hpp"
#include "object/ghost_particle_system.hpp"
#include "object/gradient.hpp"
#include "object/invisible_block.hpp"
#include "object/particlesystem.hpp"
#include "object/particlesystem_interactive.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "object/pulsing_light.hpp"
#include "object/rain_particle_system.hpp"
#include "object/smoke_cloud.hpp"
#include "object/snow_particle_system.hpp"
#include "object/text_object.hpp"
#include "object/tilemap.hpp"
#include "physfs/ifile_streambuf.hpp"
#include "supertux/collision.hpp"
#include "supertux/constants.hpp"
#include "supertux/direction.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/level.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/player_status.hpp"
#include "supertux/savegame.hpp"
#include "supertux/spawn_point.hpp"
#include "supertux/tile.hpp"
#include "supertux/tile_manager.hpp"
#include "trigger/secretarea_trigger.hpp"
#include "trigger/sequence_trigger.hpp"
#include "util/file_system.hpp"
#include "util/reader_collection.hpp"
#include "util/reader_mapping.hpp"

std::unique_ptr<Sector>
SectorParser::from_reader(Level& level, const ReaderMapping& reader)
{
  std::unique_ptr<Sector> sector(new Sector(&level));
  SectorParser parser(*sector);
  parser.parse(reader);
  return sector;
}

std::unique_ptr<Sector>
SectorParser::from_reader_old_format(Level& level, const ReaderMapping& reader)
{
  std::unique_ptr<Sector> sector(new Sector(&level));
  SectorParser parser(*sector);
  parser.parse_old_format(reader);
  return sector;
}

SectorParser::SectorParser(Sector& sector) :
  m_sector(sector)
{
}

GameObjectPtr
SectorParser::parse_object(const std::string& name_, const ReaderMapping& reader)
{
  if(name_ == "camera") {
    auto camera_ = std::make_shared<Camera>(&m_sector, "Camera");
    camera_->parse(reader);
    return camera_;
  } else if(name_ == "particles-snow") {
    auto partsys = std::make_shared<SnowParticleSystem>();
    partsys->parse(reader);
    return partsys;
  } else if(name_ == "particles-rain") {
    auto partsys = std::make_shared<RainParticleSystem>();
    partsys->parse(reader);
    return partsys;
  } else if(name_ == "particles-comets") {
    auto partsys = std::make_shared<CometParticleSystem>();
    partsys->parse(reader);
    return partsys;
  } else if(name_ == "particles-ghosts") {
    auto partsys = std::make_shared<GhostParticleSystem>();
    partsys->parse(reader);
    return partsys;
  } else if(name_ == "particles-clouds") {
    auto partsys = std::make_shared<CloudParticleSystem>();
    partsys->parse(reader);
    return partsys;
  } else if(name_ == "money") { // for compatibility with old maps
    return std::make_shared<Jumpy>(reader);
  } else {
    try {
      return ObjectFactory::instance().create(name_, reader);
    } catch(std::exception& e) {
      log_warning << e.what() << "" << std::endl;
      return {};
    }
  }
}

void
SectorParser::parse(const ReaderMapping& sector)
{
  bool has_background = false;
  auto iter = sector.get_iter();
  while(iter.next()) {
    if(iter.get_key() == "name") {
      iter.get(m_sector.name);
    } else if(iter.get_key() == "gravity") {
      iter.get(m_sector.gravity);
    } else if(iter.get_key() == "music") {
      iter.get(m_sector.music);
    } else if(iter.get_key() == "spawnpoint") {
      auto sp = std::make_shared<SpawnPoint>(iter.as_mapping());
      if (sp->name != "" && sp->pos.x >= 0 && sp->pos.y >= 0) {
        m_sector.spawnpoints.push_back(sp);
      }
    } else if(iter.get_key() == "init-script") {
      iter.get(m_sector.init_script);
    } else if(iter.get_key() == "ambient-light") {
      std::vector<float> vColor;
      sector.get( "ambient-light", vColor );
      if(vColor.size() < 3) {
        log_warning << "(ambient-light) requires a color as argument" << std::endl;
      } else {
        m_sector.ambient_light = Color( vColor );
      }
    } else {
      GameObjectPtr object = parse_object(iter.get_key(), iter.as_mapping());
      if(object) {
        if(std::dynamic_pointer_cast<Background>(object)) {
          has_background = true;
        } else if(std::dynamic_pointer_cast<Gradient>(object)) {
          has_background = true;
        }
        m_sector.add_object(object);
      }
    }
  }

  if(!has_background) {
    auto gradient = std::make_shared<Gradient>();
    gradient->set_gradient(Color(0.3, 0.4, 0.75), Color(1, 1, 1));
    m_sector.add_object(gradient);
  }

  m_sector.update_game_objects();

  if (m_sector.solid_tilemaps.empty()) {
    log_warning << "sector '" << m_sector.name << "' does not contain a solid tile layer." << std::endl;
  }

  fix_old_tiles();

  if (!m_sector.camera) {
    log_warning << "sector '" << m_sector.name << "' does not contain a camera." << std::endl;
    m_sector.update_game_objects();
    m_sector.add_object(std::make_shared<Camera>(&m_sector, "Camera"));
  }

  m_sector.update_game_objects();
  m_sector.foremost_layer = m_sector.calculate_foremost_layer();
}

void
SectorParser::parse_old_format(const ReaderMapping& reader)
{
  m_sector.name = "main";
  reader.get("gravity", m_sector.gravity);

  std::string backgroundimage;
  if (reader.get("background", backgroundimage) && (backgroundimage != "")) {
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

  if(backgroundimage != "") {
    auto background = std::make_shared<Background>();
    background->set_image(backgroundimage, bgspeed);
    m_sector.add_object(background);
  } else {
    auto gradient = std::make_shared<Gradient>();
    gradient->set_gradient(bkgd_top, bkgd_bottom);
    m_sector.add_object(gradient);
  }

  std::string particlesystem;
  reader.get("particle_system", particlesystem);
  if(particlesystem == "clouds")
    m_sector.add_object(std::make_shared<CloudParticleSystem>());
  else if(particlesystem == "snow")
    m_sector.add_object(std::make_shared<SnowParticleSystem>());
  else if(particlesystem == "rain")
    m_sector.add_object(std::make_shared<RainParticleSystem>());

  Vector startpos(100, 170);
  reader.get("start_pos_x", startpos.x);
  reader.get("start_pos_y", startpos.y);

  auto spawn = std::make_shared<SpawnPoint>();
  spawn->pos = startpos;
  spawn->name = "main";
  m_sector.spawnpoints.push_back(spawn);

  m_sector.music = "chipdisko.ogg";
  // skip reading music filename. It's all .ogg now, anyway
  /*
    reader.get("music", music);
  */
  m_sector.music = "music/" + m_sector.music;

  int width = 30, height = 15;
  reader.get("width", width);
  reader.get("height", height);

  std::vector<unsigned int> tiles;
  if(reader.get("interactive-tm", tiles)
     || reader.get("tilemap", tiles)) {
    auto tileset = TileManager::current()->get_tileset(m_sector.level->get_tileset());
    auto tilemap = std::make_shared<TileMap>(tileset);
    tilemap->set(width, height, tiles, LAYER_TILES, true);

    // replace tile id 112 (old invisible tile) with 1311 (new invisible tile)
    for(size_t x=0; x < tilemap->get_width(); ++x) {
      for(size_t y=0; y < tilemap->get_height(); ++y) {
        uint32_t id = tilemap->get_tile_id(x, y);
        if(id == 112)
          tilemap->change(x, y, 1311);
      }
    }

    if (height < 19) tilemap->resize(width, 19);
    m_sector.add_object(tilemap);
  }

  if(reader.get("background-tm", tiles)) {
    auto tileset = TileManager::current()->get_tileset(m_sector.level->get_tileset());
    auto tilemap = std::make_shared<TileMap>(tileset);
    tilemap->set(width, height, tiles, LAYER_BACKGROUNDTILES, false);
    if (height < 19) tilemap->resize(width, 19);
    m_sector.add_object(tilemap);
  }

  if(reader.get("foreground-tm", tiles)) {
    auto tileset = TileManager::current()->get_tileset(m_sector.level->get_tileset());
    auto tilemap = std::make_shared<TileMap>(tileset);
    tilemap->set(width, height, tiles, LAYER_FOREGROUNDTILES, false);

    // fill additional space in foreground with tiles of ID 2035 (lightmap/black)
    if (height < 19) tilemap->resize(width, 19, 2035);

    m_sector.add_object(tilemap);
  }

  // read reset-points (now spawn-points)
  ReaderMapping resetpoints;
  if(reader.get("reset-points", resetpoints)) {
    auto iter = resetpoints.get_iter();
    while(iter.next()) {
      if(iter.get_key() == "point") {
        Vector sp_pos;
        if(reader.get("x", sp_pos.x) && reader.get("y", sp_pos.y))
        {
          auto sp = std::make_shared<SpawnPoint>();
          sp->name = "main";
          sp->pos = sp_pos;
          m_sector.spawnpoints.push_back(sp);
        }
      } else {
        log_warning << "Unknown token '" << iter.get_key() << "' in reset-points." << std::endl;
      }
    }
  }

  // read objects
  ReaderCollection objects;
  if(reader.get("objects", objects)) {
    for(auto const& obj : objects.get_objects())
    {
      auto object = parse_object(obj.get_name(), obj.get_mapping());
      if(object) {
        m_sector.add_object(object);
      } else {
        log_warning << "Unknown object '" << obj.get_name() << "' in level." << std::endl;
      }
    }
  }

  // add a camera
  auto camera_ = std::make_shared<Camera>(&m_sector, "Camera");
  m_sector.add_object(camera_);

  m_sector.update_game_objects();

  if (m_sector.solid_tilemaps.empty()) {
    log_warning << "sector '" << m_sector.name << "' does not contain a solid tile layer." << std::endl;
  }

  fix_old_tiles();
  m_sector.update_game_objects();
}

void
SectorParser::fix_old_tiles()
{
  for(auto i = m_sector.solid_tilemaps.begin(); i != m_sector.solid_tilemaps.end(); i++) {
    TileMap* solids = *i;
    for(size_t x=0; x < solids->get_width(); ++x) {
      for(size_t y=0; y < solids->get_height(); ++y) {
        const Tile *tile = solids->get_tile(x, y);

        if (tile->get_object_name().length() > 0) {
          Vector pos = solids->get_tile_position(x, y);
          try {
            GameObjectPtr object = ObjectFactory::instance().create(tile->get_object_name(), pos, AUTO, tile->get_object_data());
            m_sector.add_object(object);
            solids->change(x, y, 0);
          } catch(std::exception& e) {
            log_warning << e.what() << "" << std::endl;
          }
        }

      }
    }
  }

  // add lights for special tiles
  for(auto i = m_sector.gameobjects.begin(); i != m_sector.gameobjects.end(); i++) {
    TileMap* tm = dynamic_cast<TileMap*>(i->get());
    if (!tm) continue;
    for(size_t x=0; x < tm->get_width(); ++x) {
      for(size_t y=0; y < tm->get_height(); ++y) {
        const Tile* tile = tm->get_tile(x, y);
        uint32_t attributes = tile->getAttributes();
        Vector pos = tm->get_tile_position(x, y);
        Vector center = pos + Vector(16, 16);

        if (attributes & Tile::FIRE) {
          if (attributes & Tile::HURTS) {
            // lava or lavaflow
            // space lights a bit
            if ((tm->get_tile(x-1, y)->getAttributes() != attributes || x%3 == 0)
                 && (tm->get_tile(x, y-1)->getAttributes() != attributes || y%3 == 0)) {
              float pseudo_rnd = (float)((int)pos.x % 10) / 10;
              m_sector.add_object(std::make_shared<PulsingLight>(center, 1.0f + pseudo_rnd, 0.8f, 1.0f, Color(1.0f, 0.3f, 0.0f, 1.0f)));
            }
          } else {
            // torch
            float pseudo_rnd = (float)((int)pos.x % 10) / 10;
            m_sector.add_object(std::make_shared<PulsingLight>(center, 1.0f + pseudo_rnd, 0.9f, 1.0f, Color(1.0f, 1.0f, 0.6f, 1.0f)));
          }
        }

      }
    }
  }
}

/* EOF */
