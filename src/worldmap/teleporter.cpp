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

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "util/reader_mapping.hpp"

namespace worldmap {

Teleporter::Teleporter(const ReaderMapping& lisp) :
  pos(),
  sprite(),
  worldmap(),
  spawnpoint(),
  automatic(false),
  message()
{
  lisp.get("x", pos.x);
  lisp.get("y", pos.y);

  std::string spritefile = "";
  if (lisp.get("sprite", spritefile)) {
    sprite = SpriteManager::current()->create(spritefile);
  }

  if(!lisp.get("worldmap", worldmap)) {
    // worldmap parameter doesn't need to be set. Ignore.
  }
  if(!lisp.get("spawnpoint", spawnpoint)) {
    // not set, use "main" spawnpoint.
  }
  if(!lisp.get("automatic", automatic)) {
    // doesn't need to be set. Don't teleport automatically.
  }
  if(!lisp.get("message", message)) {
    // Optional message not set. Ignore!
  }
}

void
Teleporter::draw(DrawingContext& context)
{
  if (sprite.get() != 0) sprite->draw(context.color(), pos*32 + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
Teleporter::update(float )
{
}

} // namespace worldmap

/* EOF */
