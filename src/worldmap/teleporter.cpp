//  SuperTux - Teleporter Worldmap Tile
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

#include "worldmap/teleporter.hpp"

#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"

namespace worldmap {

Teleporter::Teleporter(const ReaderMapping& mapping) :
  m_pos(0.0f, 0.0f),
  m_sprite(),
  m_worldmap(),
  m_sector(),
  m_spawnpoint(),
  m_automatic(false),
  m_message()
{
  mapping.get("x", m_pos.x);
  mapping.get("y", m_pos.y);

  std::string spritefile = "";
  if (mapping.get("sprite", spritefile)) {
    m_sprite = SpriteManager::current()->create(spritefile);
  }

  mapping.get("worldmap", m_worldmap);
  mapping.get("sector", m_sector);
  mapping.get("spawnpoint", m_spawnpoint);
  mapping.get("automatic", m_automatic);
  mapping.get("message", m_message);
}

void
Teleporter::draw(DrawingContext& context)
{
  if (m_sprite) {
    m_sprite->draw(context.color(), m_pos * 32.0f + Vector(16, 16), LAYER_OBJECTS - 1);
  }
}

void
Teleporter::update(float )
{
}

} // namespace worldmap

/* EOF */
