//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_STAR_HPP
#define HEADER_SUPERTUX_OBJECT_STAR_HPP

#include "object/moving_sprite.hpp"
#include "supertux/physic.hpp"

/** When a badguy melts, it creates this object. */

class WaterDrop : public MovingSprite
{
public:
  WaterDrop(const Vector& pos, std::string sprite_path_, Vector velocity);

  virtual void update(float elapsed_time);
  virtual void collision_solid(const CollisionHit& hit);
  virtual HitResponse collision(GameObject& other, const CollisionHit& );

private:
  Physic physic;

  typedef enum {
    WDS_FALLING,
    WDS_SPLASH,
    WDS_PUDDLE
  }WaterDropState;

  WaterDropState wd_state;

  std::string sprite_path;
};

#endif

/* EOF */
