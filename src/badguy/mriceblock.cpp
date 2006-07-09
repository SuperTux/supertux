//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "mriceblock.hpp"
#include "object/block.hpp"

namespace {
  const float WALKSPEED = 80;
  const float KICKSPEED = 500;
  const int MAXSQUISHES = 10;
}

MrIceBlock::MrIceBlock(const lisp::Lisp& reader)
  : BadGuy(reader, "images/creatures/mr_iceblock/mr_iceblock.sprite"), ice_state(ICESTATE_NORMAL), squishcount(0)
{
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
}

MrIceBlock::MrIceBlock(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/mr_iceblock/mr_iceblock.sprite"), ice_state(ICESTATE_NORMAL), squishcount(0)
{
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
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
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
  set_state(ICESTATE_NORMAL);
}

void
MrIceBlock::active_update(float elapsed_time)
{
  if(ice_state == ICESTATE_GRABBED)
    return;

  if(ice_state == ICESTATE_FLAT && flat_timer.check()) {
    set_state(ICESTATE_NORMAL);
  }

  if (ice_state == ICESTATE_NORMAL && might_fall(601))
  {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  BadGuy::active_update(elapsed_time);
}

void
MrIceBlock::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) { // floor or roof
    physic.set_velocity_y(0);
    return;
  }

  // hit left or right
  switch(ice_state) {
    case ICESTATE_NORMAL:
      if(hit.right && dir == RIGHT) {
        dir = LEFT;
      } else if(hit.left && dir == LEFT) {
        dir = RIGHT;
      }
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(-physic.get_velocity_x());       
      break;
    case ICESTATE_KICKED: {
#if 0
      // TODO move these into bonusblock class
      BonusBlock* bonusblock = dynamic_cast<BonusBlock*> (&object);
      if(bonusblock) {
        bonusblock->try_open();
      }
      Brick* brick = dynamic_cast<Brick*> (&object);
      if(brick) {
        brick->try_break();
      }
#endif
      if(hit.right && dir == RIGHT) {
        dir = LEFT;
        sound_manager->play("sounds/iceblock_bump.wav", get_pos());
        physic.set_velocity_x(-KICKSPEED);
      } else if(hit.left && dir == LEFT) {
        dir = RIGHT;
        sound_manager->play("sounds/iceblock_bump.wav", get_pos());
        physic.set_velocity_x(KICKSPEED);
      }
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      break;
    }
    case ICESTATE_FLAT:
      physic.set_velocity_x(0);
      break;
    case ICESTATE_GRABBED:
      break;
  }
}

HitResponse
MrIceBlock::collision(GameObject& object, const CollisionHit& hit)
{
  if(ice_state == ICESTATE_GRABBED)
    return FORCE_MOVE;

  return BadGuy::collision(object, hit);
}

HitResponse
MrIceBlock::collision_player(Player& player, const CollisionHit& hit)
{
  if(ice_state == ICESTATE_GRABBED)
    return FORCE_MOVE;

  // handle kicks from left or right side
  if(ice_state == ICESTATE_FLAT && get_state() == STATE_ACTIVE) {
    if(hit.left) {
      dir = RIGHT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    } else if(hit.right) {
      dir = LEFT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    }
  }
  
  return BadGuy::collision_player(player, hit);
}

HitResponse
MrIceBlock::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch(ice_state) {
    case ICESTATE_NORMAL:
      if(hit.left || hit.right) {
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

      set_state(ICESTATE_FLAT);
      break;
    case ICESTATE_FLAT:
      if(player.get_pos().x < get_pos().x) {
        dir = RIGHT;
      } else {
        dir = LEFT;
      }
      set_state(ICESTATE_KICKED);
      break;
    case ICESTATE_GRABBED:
      assert(false);
      break;
  }

  player.bounce(*this);
  return true;
}

void
MrIceBlock::set_state(IceState state)
{
  if(ice_state == state)
    return;
  
  if(state == ICESTATE_FLAT)
    flags |= FLAG_PORTABLE;
  else
    flags &= ~FLAG_PORTABLE;

  switch(state) {
    case ICESTATE_NORMAL:
      physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
      sprite->set_action(dir == LEFT ? "left" : "right");
      break;
    case ICESTATE_FLAT:
      sound_manager->play("sounds/stomp.wav", get_pos());
      physic.set_velocity_x(0);
      physic.set_velocity_y(0); 
      
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      flat_timer.start(4);
      break;
    case ICESTATE_KICKED:
      sound_manager->play("sounds/kick.wav", get_pos());

      physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
      // we should slide above 1 block holes now...
      bbox.set_size(34, 31.8);
      break;
    case ICESTATE_GRABBED:
      flat_timer.stop();
      break;
    default:
      assert(false);
  }
  ice_state = state;
}

void
MrIceBlock::grab(MovingObject&, const Vector& pos, Direction dir)
{
  movement = pos - get_pos();
  this->dir = dir;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
  set_state(ICESTATE_GRABBED);
  set_group(COLGROUP_DISABLED);
}

void
MrIceBlock::ungrab(MovingObject& , Direction dir)
{
  this->dir = dir;
  set_state(ICESTATE_KICKED);
  set_group(COLGROUP_MOVING);
}

IMPLEMENT_FACTORY(MrIceBlock, "mriceblock")
