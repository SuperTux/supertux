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

#ifndef HEADER_SUPERTUX_OBJECT_FLOWER_HPP
#define HEADER_SUPERTUX_OBJECT_FLOWER_HPP

#include "sprite/sprite.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/player_status.hpp"

class Flower final : public MovingObject
{
  friend class FlipLevelTransformer;

public:
  Flower(BonusType type);
  virtual bool is_saveable() const override {
    return false;
  }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;
  virtual void on_flip(float height) override;

private:
  BonusType type;
  SpritePtr sprite;
  Flip flip;

  SpritePtr lightsprite;

private:
  Flower(const Flower&) = delete;
  Flower& operator=(const Flower&) = delete;
};

#endif

/* EOF */
