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

#include "object/block.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "object/coin.hpp"
#include "object/flower.hpp"
#include "object/growup.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "supertux/constants.hpp"

static const float BOUNCY_BRICK_MAX_OFFSET = 8;
static const float BOUNCY_BRICK_SPEED = 90;
static const float EPSILON = .0001f;
static const float BUMP_ROTATION_ANGLE = 10;

Block::Block(SpritePtr newsprite) :
  sprite(newsprite),
  bouncing(false),
  breaking(false),
  bounce_dir(0),
  bounce_offset(0),
  original_y(-1)
{
  bbox.set_size(32, 32.1f);
  set_group(COLGROUP_STATIC);
  sound_manager->preload("sounds/upgrade.wav");
  sound_manager->preload("sounds/brick.wav");
}

Block::~Block()
{
}

HitResponse
Block::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    if(player->get_bbox().get_top() > get_bbox().get_bottom() - SHIFT_DELTA) {
      hit(*player);
    }
  }

  // only interact with other objects if...
  //   1) we are bouncing
  //   2) the object is not portable (either never or not currently)
  //   3) the object is being hit from below (baguys don't get killed for activating boxes)
  Portable* portable = dynamic_cast<Portable*> (&other);
  MovingObject* moving_object = dynamic_cast<MovingObject*> (&other);
  bool is_portable = ((portable != 0) && portable->is_portable());
  bool hit_mo_from_below = ((moving_object == 0) || (moving_object->get_bbox().get_bottom() < (get_bbox().get_top() + SHIFT_DELTA)));
  if(bouncing && !is_portable && hit_mo_from_below) {

    // Badguys get killed
    BadGuy* badguy = dynamic_cast<BadGuy*> (&other);
    if(badguy) {
      badguy->kill_fall();
    }

    // Coins get collected
    Coin* coin = dynamic_cast<Coin*> (&other);
    if(coin) {
      coin->collect();
    }

    //Eggs get jumped
    GrowUp* growup = dynamic_cast<GrowUp*> (&other);
    if(growup) {
      growup->do_jump();
    }

  }

  return FORCE_MOVE;
}

void
Block::update(float elapsed_time)
{
  if(!bouncing)
    return;

  float offset = original_y - get_pos().y;
  if(offset > BOUNCY_BRICK_MAX_OFFSET) {
    bounce_dir = BOUNCY_BRICK_SPEED;
    movement = Vector(0, bounce_dir * elapsed_time);
    if(breaking){
      break_me();
    }
  } else if(offset < BOUNCY_BRICK_SPEED * elapsed_time && bounce_dir > 0) {
    movement = Vector(0, offset);
    bounce_dir = 0;
    bouncing = false;
    sprite->set_angle(0);
  } else {
    movement = Vector(0, bounce_dir * elapsed_time);
  }
}

void
Block::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS+1);
}

void
Block::start_bounce(GameObject* hitter)
{
  if(original_y == -1){
    original_y = bbox.p1.y;
  }
  bouncing = true;
  bounce_dir = -BOUNCY_BRICK_SPEED;
  bounce_offset = 0;

  MovingObject* hitter_mo = dynamic_cast<MovingObject*>(hitter);
  if (hitter_mo) {
    float center_of_hitter = hitter_mo->get_bbox().get_middle().x;
    float offset = (get_bbox().get_middle().x - center_of_hitter)*2 / get_bbox().get_width();
    sprite->set_angle(BUMP_ROTATION_ANGLE*offset);
  }
}

void
Block::start_break(GameObject* hitter)
{
  start_bounce(hitter);
  breaking = true;
}

/* EOF */
