//  SuperTux
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "worldmap/level_tile.hpp"

#include <physfs.h>

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"
#include "worldmap/worldmap.hpp"

namespace worldmap {

LevelTile::LevelTile(const std::string& basedir, const ReaderMapping& mapping) :
  GameObject(mapping),
  m_pos(0.0f, 0.0f),
  m_basedir(basedir),
  m_level_filename(),
  m_title(),
  m_auto_play(false),
  m_target_time(),
  m_extro_script(),
  m_solved(false),
  m_perfect(false),
  m_statistics(),
  m_sprite(),
  m_title_color(WorldMap::level_title_color)
{
  if (!mapping.get("level", m_level_filename)) {
    // Hack for backward compatibility with 0.5.x level
    m_level_filename = m_name;
  }

  mapping.get("x", m_pos.x);
  mapping.get("y", m_pos.y);
  mapping.get("auto-play", m_auto_play);

  std::string spritefile = "images/worldmap/common/leveldot.sprite";
  mapping.get("sprite", spritefile);
  m_sprite = SpriteManager::current()->create(spritefile);

  mapping.get("extro-script", m_extro_script);

  std::vector<float> vColor;
  if (mapping.get("color", vColor)) {
    m_title_color = Color(vColor);
  }

  if (m_basedir == "./") {
    m_basedir = "";
  }

  if (!PHYSFS_exists(FileSystem::join(m_basedir, m_level_filename).c_str()))
  {
    log_warning << "level file '" << m_level_filename
                << "' does not exist and will not be added to the worldmap" << std::endl;
    return;
  }
}

LevelTile::~LevelTile()
{
}

void
LevelTile::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), m_pos * 32 + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
LevelTile::update(float )
{
}

void
LevelTile::update_sprite_action()
{
  if (!m_solved) {
    m_sprite->set_action("default");
  } else {
    m_sprite->set_action((m_sprite->has_action("perfect") && m_perfect) ? "perfect" : "solved");
  }
}

void
LevelTile::set_solved(bool v)
{
  m_solved = v;
  update_sprite_action();
}

void
LevelTile::set_perfect(bool v)
{
  m_perfect = v;
  update_sprite_action();
}

} // namespace worldmap

/* EOF */
