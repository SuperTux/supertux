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

#include <float.h>
#include "yeti.h"
#include "object/camera.h"
#include "yeti_stalactite.h"
#include "bouncing_snowball.h"

static const float JUMP_VEL1 = 250;
static const float JUMP_VEL2 = 700;
static const float RUN_SPEED = 350;
static const float JUMP_TIME = 1.6;
static const float ANGRY_JUMP_WAIT = .5;
static const int INITIAL_HITPOINTS = 3;
static const int INITIAL_BULLET_HP = 10;

Yeti::Yeti(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(80, 120);
  sprite = sprite_manager->create("yeti");
  state = INIT;
  side = LEFT;
  hitpoints = INITIAL_HITPOINTS;
  bullet_hitpoints = INITIAL_BULLET_HP;
  sound_manager->preload_sound("yeti_gna");
  sound_manager->preload_sound("yeti_roar");  
}

Yeti::~Yeti()
{
  Mix_FreeChunk(sound_gna);
}

void
Yeti::active_action(float elapsed_time)
{
  switch(state) {
    case INIT:
      break;
    case GO_RIGHT:
      physic.set_velocity_x(RUN_SPEED);
      if(timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case GO_LEFT:
      physic.set_velocity_x(-RUN_SPEED);
      if(timer.check())
        physic.set_velocity_y(JUMP_VEL2);
      break;
    case ANGRY_JUMPING:
      if(timer.check()) {
        // jump
        sound_manager->play_sound("yeti_gna");
        physic.set_velocity_y(JUMP_VEL1);
      }
      break;
    default:
      break;
  }

  movement = physic.get_movement(elapsed_time);
}

void
Yeti::go_right()
{
  // jump and move right
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(RUN_SPEED);
  state = GO_RIGHT;
  timer.start(JUMP_TIME);
}

void
Yeti::go_left()
{
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(-RUN_SPEED);
  state = GO_LEFT;
  timer.start(JUMP_TIME);
}

void
Yeti::angry_jumping()
{
  jumpcount = 0;
  timer.start(ANGRY_JUMP_WAIT);
  state = ANGRY_JUMPING;
  physic.set_velocity_x(0);
}

void
Yeti::summon_snowball()
{
  Sector::current()->add_object(new BouncingSnowball(get_pos().x+(side == LEFT ? 64 : -64), get_pos().y, (side == LEFT ? RIGHT : LEFT)));
}

HitResponse
Yeti::collision_player(Player& player, const CollisionHit& hit)
{
  if(player.is_invincible()) {
    kill_fall();
    return ABORT_MOVE;
  }
  if(hit.normal.y > .9) {
    hitpoints--;
    bullet_hitpoints--;
    sound_manager->play_sound("yeti_roar");    
    if(collision_squished(player))
      return ABORT_MOVE;
    else if (hitpoints <= 0) {
      bullet_hitpoints = 0;
      player.kill(Player::SHRINK);
      return FORCE_MOVE;
    }
  }
  player.kill(Player::SHRINK);
  return FORCE_MOVE;
}

bool
Yeti::collision_squished(Player& player)
{
  bool result = false;
  player.bounce(*this);
  if (hitpoints <= 0) {
    bullet_hitpoints = 0;
    //sprite->set_action("dead"); 
    kill_squished(player);
    result = true;
  }
  return result;
}

void
Yeti::write(lisp::Writer& )
{
}

void
Yeti::drop_stalactite()
{
  YetiStalactite* nearest = 0;
  float dist = FLT_MAX;

  Sector* sector = Sector::current();
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    YetiStalactite* stalactite = dynamic_cast<YetiStalactite*> (*i);
    if(stalactite && stalactite->is_hanging()) {
      float sdist 
        = fabsf(stalactite->get_pos().x - sector->player->get_pos().x);
      if(sdist < dist) {
        nearest = stalactite;
        dist = sdist;
      }
    }
  }

  if(nearest)
    nearest->start_shaking();
}

HitResponse
Yeti::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
    if(state == INIT) {
      go_right();
    } else if(state == GO_LEFT && !timer.started()) {
      side = LEFT;
      summon_snowball();
      angry_jumping();
    } else if(state == GO_RIGHT && !timer.started()) {
      side = RIGHT;
      summon_snowball();
      angry_jumping();
    } else if(state == ANGRY_JUMPING) {
      if(!timer.started()) {
        // we just landed
        jumpcount++;
        // make a stalactite falling down and shake camera a bit
        Sector::current()->camera->shake(.1, 0, 10);
        drop_stalactite();
        
        // go to other side after 3 jumps
        if(jumpcount == 3) {
          if(side == LEFT)
            go_right();
          else
            go_left();
        } else {
          // jump again
          timer.start(ANGRY_JUMP_WAIT);
        }
      }
    }
  }
  
  return CONTINUE;
}

void
Yeti::kill_fall()
{
  sound_manager->play_sound("yeti_roar");  
  bullet_hitpoints--;
  if (bullet_hitpoints <= 0) {
    sound_manager->play_sound("fall", this,
                              Sector::current()->player->get_pos());
    physic.set_velocity_y(0);
    physic.enable_gravity(true);
    set_state(STATE_FALLING);
  }
}

IMPLEMENT_FACTORY(Yeti, "yeti")
