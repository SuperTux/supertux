//  SuperTux - Unstable Tile
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#ifndef HEADER_SUPERTUX_OBJECT_UNSTABLE_TILE_HPP
#define HEADER_SUPERTUX_OBJECT_UNSTABLE_TILE_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"
#include "supertux/timer.hpp"

/**
 * A block that disintegrates when stood on
 */
class UnstableTile : public MovingSprite
{
public:
  UnstableTile(const ReaderMapping& lisp);

  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);

private:
  enum State {
    STATE_NORMAL,   /**< default state */
    STATE_SHAKE,    /**< shaking, still solid */
    STATE_DISSOLVE, /**< dissolving, will turn non-solid after this */
    STATE_SLOWFALL, /**< slow fall phase (used when neither shaking nor dissolving exist */
    STATE_FALL      /**< falling down */
  };

  void startCrumbling();

private:
  void shake (void);
  void dissolve (void);
  void fall_down (void);
  void slow_fall (void);

  Physic physic;
  State state;
  float slowfall_timer;
};

#endif

/* EOF */
