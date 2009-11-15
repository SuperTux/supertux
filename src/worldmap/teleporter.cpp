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
#include <config.h>

#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"
#include "worldmap/teleporter.hpp"

namespace WorldMapNS
{

Teleporter::Teleporter(const lisp::Lisp* lisp)
  : automatic(false)
{
  lisp->get("x", pos.x);
  lisp->get("y", pos.y);

  std::string spritefile = "";
  if (lisp->get("sprite", spritefile)) {
    sprite.reset(sprite_manager->create(spritefile));
  }

  lisp->get("worldmap", worldmap);
  lisp->get("spawnpoint", spawnpoint);
  lisp->get("automatic", automatic);
  lisp->get("message", message);
}

void
Teleporter::draw(DrawingContext& context)
{
  if (sprite.get() != 0) sprite->draw(context, pos*32 + Vector(16, 16), LAYER_OBJECTS - 1);
}

void
Teleporter::update(float )
{
}

}
