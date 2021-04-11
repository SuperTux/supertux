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

#include "sprite/sprite_ptr.hpp"
#include "supertux/direction.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/physic.hpp"
#include "supertux/player_status.hpp"

class Bullet final : public MovingObject
{
public:
  Bullet(const Vector& pos, const Vector& xm, Direction dir, BonusType type);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual bool is_saveable() const override { return false; }

  virtual void backup(Writer& writer) const override;
  virtual void restore(const ReaderMapping& reader) override;

  /** Makes bullet bounce off an object (that got hit). To be called
      by the collision handler of that object. Note that the @c hit
      parameter is filled in as perceived by the object, not by the
      bullet. */
  void ricochet(GameObject& other, const CollisionHit& hit);

  BonusType get_type() const { return type; }

private:
  Physic physic;
  int life_count;
  SpritePtr sprite;
  SpritePtr lightsprite;
  BonusType type;

private:
  Bullet(const Bullet&) = delete;
  Bullet& operator=(const Bullet&) = delete;
};

#endif

/* EOF */
