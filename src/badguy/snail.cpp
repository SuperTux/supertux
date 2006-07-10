//  $Id$
//
//  SuperTux - Badguy "Snail"
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "snail.hpp"
#include "object/block.hpp"

namespace {
  const float WALKSPEED = 80;
  const float KICKSPEED = 500;
  const int MAXSQUISHES = 10;
  const float KICKSPEED_Y = -500; /**< y-velocity gained when kicked */
}

Snail::Snail(const lisp::Lisp& reader)
  : BadGuy(reader, "images/creatures/snail/snail.sprite"), state(STATE_NORMAL), squishcount(0)
{
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
  recently_changed_direction = false;
}

Snail::Snail(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/snail/snail.sprite"), state(STATE_NORMAL), squishcount(0)
{
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
  recently_changed_direction = false;
}

void
Snail::write(lisp::Writer& writer)
{
  writer.start_list("snail");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("snail");
}

void
Snail::activate()
{
  be_normal();
}

void
Snail::be_normal()
{
  state = STATE_NORMAL;
  sprite->set_action(dir == LEFT ? "left" : "right");

  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
}

void
Snail::be_flat()
{
  state = STATE_FLAT;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
  sprite->set_fps(64);
  
  physic.set_velocity_x(0);
  physic.set_velocity_y(0); 
  
  flat_timer.start(4);
}

void
Snail::be_kicked()
{
  state = STATE_KICKED_DELAY;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
  sprite->set_fps(64);

  physic.set_velocity_x(0);
  physic.set_velocity_y(0); 
 
  // start a timer to delay addition of upward movement until we are (hopefully) out from under the player
  kicked_delay_timer.start(0.05);
}


void
Snail::active_update(float elapsed_time)
{
  recently_changed_direction = false;
  switch (state) {

    case STATE_NORMAL:
      if (on_ground() && might_fall(601)) {
        if( recently_changed_direction ) break;
        recently_changed_direction = true;
	dir = (dir == LEFT ? RIGHT : LEFT);
	sprite->set_action(dir == LEFT ? "left" : "right");
	physic.set_velocity_x(-physic.get_velocity_x());
      }
      break;

    case STATE_FLAT:
      if (flat_timer.started()) {
	sprite->set_fps(64 - 15 * flat_timer.get_timegone());
      }
      if (flat_timer.check()) {
	be_normal();
      }
      break;

    case STATE_KICKED_DELAY:
      if (kicked_delay_timer.check()) {
	physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
	physic.set_velocity_y(KICKSPEED_Y);
	state = STATE_KICKED;
      }
      break;

    case STATE_KICKED:
      physic.set_velocity_x(physic.get_velocity_x() * pow(0.99, elapsed_time/0.02));
      if (fabsf(physic.get_velocity_x()) < WALKSPEED) be_normal();
      break;

  }
  BadGuy::active_update(elapsed_time);
}

void
Snail::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);

  if(hit.top || hit.bottom) { // floor or roof
    physic.set_velocity_y(0);

    switch (state) {
      case STATE_NORMAL:
      case STATE_FLAT:
      case STATE_KICKED_DELAY:
	break;
      case STATE_KICKED:
	break;
    }

    return;
  }
  // hit left or right
  switch(state) {
    
    case STATE_NORMAL:
      if( recently_changed_direction ) break;
      recently_changed_direction = true;
      dir = dir == LEFT ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(-physic.get_velocity_x());       
      break;

    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      physic.set_velocity_x(0);
      break;

    case STATE_KICKED: {
      sound_manager->play("sounds/iceblock_bump.wav", get_pos());
    
#if 0
      // TODO move this into BonusBlock code
      // open bonusblocks, crash bricks
      BonusBlock* bonusblock = dynamic_cast<BonusBlock*> (&object);
      if(bonusblock) {
        bonusblock->try_open();
      }
      Brick* brick = dynamic_cast<Brick*> (&object);
      if(brick) {
        brick->try_break();
      }
#endif
      if( recently_changed_direction ) break;
      recently_changed_direction = true;
      dir = (dir == LEFT) ? RIGHT : LEFT;
      sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");

      physic.set_velocity_x(-physic.get_velocity_x()*0.75);
      if (fabsf(physic.get_velocity_x()) < WALKSPEED) be_normal();
      break;

    }
  }
}

HitResponse
Snail::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch(state) {
    case STATE_NORMAL:
      if(hit.left || hit.right) {
        if( recently_changed_direction ) return CONTINUE;
        recently_changed_direction = true;
        dir = (dir == LEFT) ? RIGHT : LEFT;
        sprite->set_action(dir == LEFT ? "left" : "right");
        physic.set_velocity_x(-physic.get_velocity_x());               
      }
      return CONTINUE;
    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      return FORCE_MOVE;
    case STATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

bool
Snail::collision_squished(Player& player)
{
  switch(state) {

    case STATE_KICKED:
    case STATE_NORMAL:
      squishcount++;
      if(squishcount >= MAXSQUISHES) {
        kill_fall();
        return true;
      }

      sound_manager->play("sounds/stomp.wav", get_pos());
      be_flat();
      break;
      
    case STATE_FLAT:
      sound_manager->play("sounds/kick.wav", get_pos());

      if(player.get_pos().x < get_pos().x) {
        dir = RIGHT;
      } else {
        dir = LEFT;
      }
      be_kicked();
      break;

    case STATE_KICKED_DELAY:
      break;
      
  }

  player.bounce(*this);
  return true;
}

IMPLEMENT_FACTORY(Snail, "snail")
