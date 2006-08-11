//  $Id:$
//
//  SuperTux - MagicBlock
//
//  Magic Blocks are tile-like game objects that are sensitive to 
//  lighting conditions. They are rendered in a color and
//  will only be solid as long as light of the same color shines
//  on the block.
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

#include <config.h>
#include <vector>

#include "magicblock.hpp"
#include "object_factory.hpp"
#include "sprite/sprite_manager.hpp"

namespace {
  const float MIN_INTENSITY = 0.8;
  const float ALPHA_SOLID = 0.7;
  const float ALPHA_NONSOLID = 0.3;
}

MagicBlock::MagicBlock(const lisp::Lisp& lisp)
	: MovingSprite(lisp, "images/objects/magicblock/magicblock.sprite"),
        is_solid(false), light(1.0f,1.0f,1.0f)
{
  set_group(COLGROUP_STATIC);
  //get color from lisp
  std::vector<float> vColor;
  lisp.get_vector("color", vColor );
  color = Color( vColor );

  //all alpha to make the sprite still visible
  color.alpha = ALPHA_SOLID;

  //set trigger
  trigger_red = (color.red == 1.0f ? MIN_INTENSITY : 0);
  trigger_green = (color.green == 1.0f ? MIN_INTENSITY : 0);
  trigger_blue = (color.blue == 1.0f ? MIN_INTENSITY : 0);

  center =  Vector((get_bbox().p1.x+get_bbox().p2.x)/2,  
                   (get_bbox().p1.y+get_bbox().p2.y)/2);
}

void
MagicBlock::update(float /*elapsed_time*/)
{
  if(light.red >= trigger_red && light.green >= trigger_green 
      && light.blue >= trigger_blue) {
    is_solid = true;
  } else {
    is_solid = false;
  }

  //Update Sprite.
  if(is_solid) {
    color.alpha = ALPHA_SOLID;
    sprite->set_action("solid");
  } else {
    color.alpha = ALPHA_NONSOLID;
    sprite->set_action("normal");
  }
}

void
MagicBlock::draw(DrawingContext& context){
  //Ask for update about lightmap at center of this block
  context.get_light( center, &light );

  //Draw the Sprite.
  MovingSprite::draw(context);
  //Add the color.
  context.draw_filled_rect( get_bbox(), color, layer);
}

HitResponse
MagicBlock::collision(GameObject& /*other*/, const CollisionHit& /*hit*/)
{
  if(is_solid) {
    return SOLID;
  } else {
    return PASSTHROUGH;
  }
}

IMPLEMENT_FACTORY(MagicBlock, "magicblock");
