//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __SKULL_TILE_H__
#define __SKULL_TILE_H__

#include "object/moving_sprite.hpp"
#include "physic.hpp"
#include "timer.hpp"

class Player;

/** A tile that starts falling down if tux stands to long on it */
class SkullTile : public MovingSprite, private UsesPhysic
{
public:
  SkullTile(const lisp::Lisp& lisp);
  virtual SkullTile* clone() const { return new SkullTile(*this); }

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void draw(DrawingContext& context);

private:
  Timer timer;
  bool hit;
  bool falling;
};

#endif
