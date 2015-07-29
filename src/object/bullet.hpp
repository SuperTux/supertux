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

#ifndef HEADER_SUPERTUX_OBJECT_BULLET_HPP
#define HEADER_SUPERTUX_OBJECT_BULLET_HPP

#include "sprite/sprite.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/physic.hpp"
#include "supertux/player_status.hpp"

class Bullet : public MovingObject
{
public:
  Bullet(const Vector& pos, float xm, int dir, BonusType type);
  ~Bullet();

  void update(float elapsed_time);
  void draw(DrawingContext& context);
  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual bool do_save() {
    return false;
  }

  /**
   * Makes bullet bounce off an object (that got hit).
   * To be called by the collision handler of that object.
   * Note that the @c hit parameter is filled in as perceived by the object, not by the bullet.
   */
  void ricochet(GameObject& other, const CollisionHit& hit);

  BonusType get_type() const
  {
    return type;
  }

private:
  Physic physic;
  int life_count;
  SpritePtr sprite;
  Color light;
  SpritePtr lightsprite;
  BonusType type;
};

#endif

/* EOF */
