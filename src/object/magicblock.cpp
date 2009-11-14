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

#include "object/camera.hpp"
#include "magicblock.hpp"
#include "object_factory.hpp"
#include "sprite/sprite_manager.hpp"
#include "sector.hpp"
#include "main.hpp"
#include "sprite/sprite.hpp"

namespace {
  const float MIN_INTENSITY = 0.8f;
  const float ALPHA_SOLID = 0.7f;
  const float ALPHA_NONSOLID = 0.3f;
  const float MIN_SOLIDTIME = 1.0f;
  const float SWITCH_DELAY = 0.1f; /**< seconds to wait for stable conditions until switching solidity */
}

MagicBlock::MagicBlock(const lisp::Lisp& lisp)
        : MovingSprite(lisp, "images/objects/magicblock/magicblock.sprite"),
        is_solid(false), solid_time(0), switch_delay(0), light(1.0f,1.0f,1.0f)
{
  set_group(COLGROUP_STATIC);
  //get color from lisp
  std::vector<float> vColor;
  lisp.get("color", vColor );
  color = Color( vColor );

  //all alpha to make the sprite still visible
  color.alpha = ALPHA_SOLID;

  //set trigger
  if(color.red == 0 && color.green == 0 && color.blue == 0) { //is it black?
    black = true;
    trigger_red = MIN_INTENSITY;
    trigger_green = MIN_INTENSITY;
    trigger_blue = MIN_INTENSITY;
  } else {
    black = false;
    trigger_red = (color.red == 1.0f ? MIN_INTENSITY : 0);
    trigger_green = (color.green == 1.0f ? MIN_INTENSITY : 0);
    trigger_blue = (color.blue == 1.0f ? MIN_INTENSITY : 0);
  }

  center = get_bbox().get_middle();
}

void
MagicBlock::update(float elapsed_time)
{
  //Check if center of this block is on screen.
  //Don't update if not, because there is no light off screen.
  float screen_left = Sector::current()->camera->get_translation().x;
  float screen_top = Sector::current()->camera->get_translation().y;
  float screen_right = screen_left+ SCREEN_WIDTH;
  float screen_bottom = screen_top + SCREEN_HEIGHT;
  if((center.x > screen_right ) || ( center.y > screen_bottom) ||
     ( center.x < screen_left) || ( center.y < screen_top)) {
    switch_delay = SWITCH_DELAY;
    return;
  }

  bool lighting_ok;
  if(black) {
    lighting_ok = (light.red >= trigger_red || light.green >= trigger_green
      || light.blue >= trigger_blue);
  }else{
    lighting_ok = (light.red >= trigger_red && light.green >= trigger_green
      && light.blue >= trigger_blue);
  }

  // overrule lighting_ok if switch_delay has not yet passed
  if (lighting_ok == is_solid) {
    switch_delay = SWITCH_DELAY;
  } else {
    if (switch_delay > 0) {
      lighting_ok = is_solid;
      switch_delay -= elapsed_time;
    }
  }

  if (lighting_ok) {
    // lighting suggests going solid

    if (!is_solid) {
      if (Sector::current()->is_free_of_movingstatics(get_bbox(), this)) {
        is_solid = true;
        solid_time = 0;
        switch_delay = SWITCH_DELAY;
      }
    }
  } else {
    // lighting suggests going nonsolid

    if( solid_time >= MIN_SOLIDTIME ){
      is_solid = false;
    }
  }

  //Update Sprite.
  if(is_solid) {
    solid_time+=elapsed_time;
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

bool
MagicBlock::collides(GameObject& /*other*/, const CollisionHit& /*hit*/)
{
  return is_solid;
}

HitResponse
MagicBlock::collision(GameObject& /*other*/, const CollisionHit& /*hit*/)
{
  return SOLID;
}

IMPLEMENT_FACTORY(MagicBlock, "magicblock");
