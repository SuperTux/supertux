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

#ifndef HEADER_SUPERTUX_OBJECT_SKULL_TILE_HPP
#define HEADER_SUPERTUX_OBJECT_SKULL_TILE_HPP

#include "object/moving_sprite.hpp"

class Player;

/** A tile that starts falling down if tux stands to long on it */
class SkullTile : public MovingSprite
{
public:
  SkullTile(const Reader& lisp);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void draw(DrawingContext& context);

private:
  Physic physic;
  Timer timer;
  bool hit;
  bool falling;
};

#endif

/* EOF */
