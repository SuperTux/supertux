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

#include "worldmap/special_tile.hpp"

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/log.hpp"
#include "util/reader_mapping.hpp"

namespace worldmap {

SpecialTile::SpecialTile(const ReaderMapping& mapping) :
  m_pos(0.0f, 0.0f),
  m_sprite(),
  m_map_message(),
  m_passive_message(false),
  m_script(),
  m_invisible(false),
  m_apply_action_north(true),
  m_apply_action_east(true),
  m_apply_action_south(true),
  m_apply_action_west(true)
{
  if (!mapping.get("x", m_pos.x)) {
    log_warning << "X coordinate of special tile not set, defaulting to 0" << std::endl;
  }
  if (!mapping.get("y", m_pos.y)) {
    log_warning << "Y coordinate of special tile not set, defaulting to 0" << std::endl;
  }
  if (!mapping.get("invisible-tile", m_invisible)) {
    // Ignore attribute if it's not specified. Tile is visible.
  }

  if (!m_invisible) {
    std::string spritefile = "";
    if (!mapping.get("sprite", spritefile)) {
      log_warning << "No sprite specified for visible special tile." << std::endl;
    }
    m_sprite = SpriteManager::current()->create(spritefile);
  }

  if (!mapping.get("map-message", m_map_message)) {
    // Ignore attribute if it's not specified. No map message set.
  }
  if (!mapping.get("passive-message", m_passive_message)) {
    // Ignore attribute if it's not specified. No passive message set.
  }
  if (!mapping.get("script", m_script)) {
    // Ignore attribute if it's not specified. No script set.
  }

  std::string apply_direction;
  if (!mapping.get("apply-to-direction", apply_direction)) {
    // Ignore attribute if it's not specified. Applies to all directions.
  }
  if (!apply_direction.empty()) {
    m_apply_action_north = false;
    m_apply_action_south = false;
    m_apply_action_east = false;
    m_apply_action_west = false;
    if (apply_direction.find("north") != std::string::npos)
      m_apply_action_north = true;
    if (apply_direction.find("south") != std::string::npos)
      m_apply_action_south = true;
    if (apply_direction.find("east") != std::string::npos)
      m_apply_action_east = true;
    if (apply_direction.find("west") != std::string::npos)
      m_apply_action_west = true;
  }
}

SpecialTile::~SpecialTile()
{
}

void
SpecialTile::draw(DrawingContext& context)
{
  if (m_invisible)
    return;

  m_sprite->draw(context.color(), m_pos*32.0f + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
SpecialTile::update(float )
{
}

}

/* EOF */
