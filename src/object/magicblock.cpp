//  SuperTux - MagicBlock
//
//  Magic Blocks are tile-like game objects that are sensitive to
//  lighting conditions. They are rendered in a color and
//  will only be solid as long as light of the same color shines
//  on the block.
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

#include "object/magicblock.hpp"

#include "object/camera.hpp"
#include "sprite/sprite.hpp"
#include "supertux/globals.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
const float MIN_INTENSITY = 0.8f;
const float ALPHA_SOLID = 0.7f;
const float ALPHA_NONSOLID = 0.3f;
const float MIN_SOLIDTIME = 1.0f;
const float SWITCH_DELAY = 0.06f; /**< seconds to wait for stable conditions until switching solidity */
}

MagicBlock::MagicBlock(const ReaderMapping& lisp) :
  MovingSprite(lisp, "images/objects/magicblock/magicblock.sprite"),
  is_solid(false),
  trigger_red(),
  trigger_green(),
  trigger_blue(),
  solid_time(0),
  switch_delay(0),
  color(),
  light(1.0f,1.0f,1.0f),
  center(),
  black()
{
  set_group(COLGROUP_STATIC);
  //get color from lisp
  std::vector<float> vColor;
  if (lisp.get("color", vColor )) {
    color = Color( vColor );
  } else {
    color = Color(0, 0, 0);
  }

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
    trigger_red = color.red;
    trigger_green = color.green;
    trigger_blue = color.blue;
  }

  center = bbox.get_middle();
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
      if (Sector::current()->is_free_of_movingstatics(bbox, this)) {
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
    set_group(COLGROUP_STATIC);
  } else {
    color.alpha = ALPHA_NONSOLID;
    sprite->set_action("normal");
    set_group(COLGROUP_DISABLED);
  }
}

void
MagicBlock::draw(DrawingContext& context){
  //Ask for update about lightmap at center of this block
  context.get_light( center, &light );

  //Draw the Sprite.
  MovingSprite::draw(context);
  //Add the color.
  context.draw_filled_rect( bbox, color, layer);
}

bool
MagicBlock::collides(GameObject& /*other*/, const CollisionHit& /*hit*/) const
{
  return is_solid;
}

HitResponse
MagicBlock::collision(GameObject& /*other*/, const CollisionHit& /*hit*/)
{
  return FORCE_MOVE;
}

/* EOF */
