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

#define EXPLOSION_STRENGTH_DEFAULT (1464.8f * 32.0f * 32.0f)
#define EXPLOSION_STRENGTH_NEAR (150.0f * 32.0f * 32.0f)

/** Just your average explosion - goes boom, hurts Tux */
class Explosion final : public MovingSprite
{
public:
  /** Create new Explosion centered(!) at @c pos */
  Explosion(const Vector& pos, float push_strength, int num_particles=100);
  Explosion(const ReaderMapping& reader);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual bool is_saveable() const override { return false; }

  bool hurts() const { return hurt; }
  void hurts (bool val) { hurt = val; }

private:
  /** plays sound, starts animation */
  void explode();

private:
  enum State {
    STATE_WAITING,
    STATE_EXPLODING
  };

private:
  bool hurt;
  float push_strength;
  int num_particles;
  State state;
  SpritePtr lightsprite;

private:
  Explosion(const Explosion&) = delete;
  Explosion& operator=(const Explosion&) = delete;
};

#endif

/* EOF */
