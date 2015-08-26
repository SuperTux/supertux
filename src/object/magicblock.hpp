//  SuperTux - MagicBlock
//
//  Magic Blocks are tile-like game objects that are sensitive to
//  lighting conditions. They are rendered in a color and
//  will only be solid as long as light of the same color shines
//  on the block. The black block becomes solid, if any kind of
//  light is above MIN_INTENSITY.
//
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_MAGICBLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_MAGICBLOCK_HPP

#include "object/moving_sprite.hpp"

class MagicBlock: public MovingSprite
{
public:
  MagicBlock(const Reader& reader);
  virtual void save(lisp::Writer& writer);

  bool collides(GameObject& other, const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void draw(DrawingContext& context);
  virtual std::string get_class() {
    return "magicblock";
  }

  virtual ObjectSettings get_settings();

private:
  bool is_solid;
  float trigger_red;
  float trigger_green;
  float trigger_blue;
  float solid_time;
  float switch_delay; /**< seconds until switching solidity */
  Color color;
  Color light;
  Vector center;
  bool black;
};

#endif

/* EOF */
