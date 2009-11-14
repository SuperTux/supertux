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
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_MAGICBLOCK_H
#define SUPERTUX_MAGICBLOCK_H

#include "moving_sprite.hpp"
#include "lisp/lisp.hpp"

class MagicBlock: public MovingSprite
{
public:
  MagicBlock(const lisp::Lisp& reader);

  bool collides(GameObject& other, const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);
  void update(float elapsed_time);
  void draw(DrawingContext& context);

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
