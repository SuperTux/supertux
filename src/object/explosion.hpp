//  SuperTux -- Explosion object
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_EXPLOSION_HPP
#define HEADER_SUPERTUX_OBJECT_EXPLOSION_HPP

#include "object/moving_sprite.hpp"

/**
 * Just your average explosion - goes boom, hurts Tux
 */
class Explosion : public MovingSprite
{
public:
  /**
   * Create new Explosion centered(!) at @c pos
   */
  Explosion(const Vector& pos);
  Explosion(const ReaderMapping& reader);

  void update(float elapsed_time);
  void draw(DrawingContext& context);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  virtual bool is_saveable() const {
    return false;
  }

  bool hurts() const
  {
    return this->hurt;
  }

  void hurts (bool val)
  {
    this->hurt = val;
  }

  bool pushes() const
  {
    return this->push;
  }

  void pushes (bool val)
  {
    this->push = val;
  }

protected:
  /**
   * plays sound, starts animation
   */
  void explode();

private:
  enum State {
    STATE_WAITING,
    STATE_EXPLODING
  };
  bool hurt;
  bool push;
  State state;
  Color light;
  SpritePtr lightsprite;

};

#endif

/* EOF */
