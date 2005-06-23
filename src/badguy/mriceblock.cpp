//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include "mriceblock.h"
#include "object/block.h"

static const float WALKSPEED = 80;
static const float KICKSPEED = 500;
static const int MAXSQUISHES = 10;

MrIceBlock::MrIceBlock(const lisp::Lisp& reader)
  : ice_state(ICESTATE_NORMAL), squishcount(0)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mriceblock");
  set_direction = false;
}

MrIceBlock::MrIceBlock(float pos_x, float pos_y, Direction d)
  : ice_state(ICESTATE_NORMAL), squishcount(0)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("mriceblock");
  set_direction = true;
  initial_direction = d;
}

void
MrIceBlock::write(lisp::Writer& writer)
{
  writer.start_list("mriceblock");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("mriceblock");
}

void
MrIceBlock::activate()
{
  if (set_direction) {dir = initial_direction;}
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

void
MrIceBlock::active_update(float elapsed_time)
{
  if(ice_state == ICESTATE_FLAT && flat_timer.check()) {
    ice_state = ICESTATE_NORMAL;
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "left" : "right");
  }
  BadGuy::active_update(elapsed_time);
}

HitResponse
MrIceBlock::collision_solid(GameObject& object, const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // floor or roof
    physic.set_velocity_y(0);
    return CONTINUE;
  }
  // hit left or right
  switch(ice_state) {
    case ICESTATE_NORMAL:
      dir = dir == LEFT ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(-physic.get_velocity_x());       
      break;
    case ICESTATE_KICKED: {
      BonusBlock* bonusblock = dynamic_cast<BonusBlock*> (&object);
      if(bonusblock) {
        bonusblock->try_open();
      }
      Brick* brick = dynamic_cast<Brick*> (&object);
      if(brick) {
        brick->try_break();
      }
      
      dir = dir == LEFT ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      physic.set_velocity_x(-physic.get_velocity_x());
      sound_manager->play("ricochet", get_pos());
      break;
    }
    case ICESTATE_FLAT:
      physic.set_velocity_x(0);
      break;
  }

  return CONTINUE;
}

HitResponse
MrIceBlock::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch(ice_state) {
    case ICESTATE_NORMAL:
      if(fabsf(hit.normal.x) > .8) {
        dir = dir == LEFT ? RIGHT : LEFT;
        sprite->set_action(dir == LEFT ? "left" : "right");
        physic.set_velocity_x(-physic.get_velocity_x());               
      }
      return CONTINUE;
    case ICESTATE_FLAT:
      return FORCE_MOVE;
    case ICESTATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

bool
MrIceBlock::collision_squished(Player& player)
{
  switch(ice_state) {
    case ICESTATE_KICKED:
    case ICESTATE_NORMAL:
      squishcount++;
      if(squishcount >= MAXSQUISHES) {
        kill_fall();
        return true;
      }

      // flatten
      sound_manager->play("stomp", get_pos());
      physic.set_velocity_x(0);
      physic.set_velocity_y(0); 
      
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      flat_timer.start(4);
      ice_state = ICESTATE_FLAT;
      break;
    case ICESTATE_FLAT:
      // kick
      sound_manager->play("kick", get_pos());

      if(player.get_pos().x < get_pos().x) {
        dir = RIGHT;
      } else {
        dir = LEFT;
      }
      physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      ice_state = ICESTATE_KICKED;
      break;
  }

  player.bounce(*this);
  return true;
}

IMPLEMENT_FACTORY(MrIceBlock, "mriceblock")
