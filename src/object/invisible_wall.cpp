//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "invisible_wall.hpp"
#include "lisp/lisp.hpp"
#include "supertux/object_factory.hpp"
#include "sprite/sprite.hpp"

InvisibleWall::InvisibleWall(const lisp::Lisp& lisp)
        : MovingSprite(lisp, "images/objects/invisible/invisible.sprite", LAYER_TILES, COLGROUP_STATIC), width(32), height(32)
{
        lisp.get("width", width);
        lisp.get("height", height);
        bbox.set_size(width, height);
}

HitResponse
InvisibleWall::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(InvisibleWall, "invisible_wall");
