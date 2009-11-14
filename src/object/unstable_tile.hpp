//  SuperTux - Unstable Tile
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __UNSTABLE_TILE_H__
#define __UNSTABLE_TILE_H__

#include "object/moving_sprite.hpp"
#include "lisp/lisp.hpp"
#include "physic.hpp"
#include "timer.hpp"

/**
 * A block that disintegrates when stood on
 */
class UnstableTile : public MovingSprite, public UsesPhysic
{
public:
  UnstableTile(const lisp::Lisp& lisp);
  virtual UnstableTile* clone() const { return new UnstableTile(*this); }

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);

private:
  enum State {
    STATE_NORMAL,        /**< default state */
    STATE_CRUMBLING,     /**< crumbling, still solid */
    STATE_DISINTEGRATING /**< disintegrating, no longer solid */
  };
  State state;
};

#endif
