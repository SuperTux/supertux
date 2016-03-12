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

class Flower : public MovingObject
{
public:
  Flower(BonusType type);
  ~Flower();
  virtual bool do_save() const {
    return false;
  }

  virtual void update(float elapsed_time);
  virtual void draw(DrawingContext& context);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);

private:
  BonusType type;
  SpritePtr sprite;
  DrawingEffect drawing_effect;

private:
  Flower(const Flower&);
  Flower& operator=(const Flower&);
  Color light;
  SpritePtr lightsprite;

  friend class FlipLevelTransformer;
};

#endif

/* EOF */
