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
#include <sstream>
#include <memory>
#include "yeti.hpp"
#include "object/camera.hpp"
#include "yeti_stalactite.hpp"
#include "bouncing_snowball.hpp"
#include "game_session.hpp"

static const float JUMP_VEL1 = 250;
static const float JUMP_VEL2 = 700;
static const float RUN_SPEED = 350;
static const float JUMP_TIME = 1.6;
static const float ANGRY_JUMP_WAIT = .5;
/// the time we are safe when tux just hit us
static const float SAFE_TIME = .5;
static const int INITIAL_HITPOINTS = 3;

Yeti::Yeti(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(60, 90);
  sprite = sprite_manager->create("images/creatures/yeti/yeti.sprite");
  sprite->set_action("right");
  state = INIT;
  side = LEFT;
  hit_points = INITIAL_HITPOINTS;
  reader.get("dead-script", dead_script);
  countMe = false;
}

Yeti::~Yeti()
{
}

void
Yeti::draw(DrawingContext& context)
{
  // we blink when we are safe
  if(safe_timer.started() && size_t(game_time*40)%2)
    return;

  BadGuy::draw(context);
}

void
Yeti::active_update(float elapsed_time)
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
        sound_manager->play("sounds/yeti_gna.wav");
        physic.set_velocity_y(JUMP_VEL1);
        if (side == LEFT)  // on the left, facing Tux who is on the right
          sprite->set_action("jump-right");
        else
          sprite->set_action("jump-left");
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
  sprite->set_action("right");
  physic.set_velocity_y(JUMP_VEL1);
  physic.set_velocity_x(RUN_SPEED);
  state = GO_RIGHT;
  timer.start(JUMP_TIME);
}

void
Yeti::go_left()
{
  sprite->set_action("left");
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

bool
Yeti::collision_squished(Player& player)
{
  if(safe_timer.started())
    return true;

  player.bounce(*this);
  sound_manager->play("sounds/yeti_roar.wav");
  hit_points--;
  if(hit_points <= 0) {
    sprite->set_action("dead");
    kill_squished(player);

    // start script
    if(dead_script != "") {
      std::istringstream stream(dead_script);
      Sector::current()->run_script(stream, "Yeti - dead-script");
    }
  } else {
    safe_timer.start(SAFE_TIME);
  }
  
  return true;
}

void
Yeti::kill_fall()
{
  // shooting bullets or being invincible won't work :)
}

void
Yeti::write(lisp::Writer& writer)
{
  writer.start_list("yeti");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  if(dead_script != "") {
    writer.write_string("dead-script", dead_script);
  }

  writer.end_list("yeti");
}

void
Yeti::drop_stalactite()
{
  YetiStalactite* nearest = 0;
  float dist = FLT_MAX;

  Player* player = this->get_nearest_player();
  if (!player) return;

  Sector* sector = Sector::current();
  for(Sector::GameObjects::iterator i = sector->gameobjects.begin();
      i != sector->gameobjects.end(); ++i) {
    YetiStalactite* stalactite = dynamic_cast<YetiStalactite*> (*i);
    if(stalactite && stalactite->is_hanging()) {
      float sdist 
        = fabsf(stalactite->get_pos().x - player->get_pos().x);
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
      sprite->set_action("stand-right");
      angry_jumping();
    } else if(state == GO_RIGHT && !timer.started()) {
      side = RIGHT;
      summon_snowball();
      sprite->set_action("stand-left");
      angry_jumping();
    } else if(state == ANGRY_JUMPING) {
      if(!timer.started()) {
        // we just landed
        if (side == LEFT)  // standing on the left, facing Tux who is on the right
 	      sprite->set_action("stand-right");
 	    else
 	      sprite->set_action("stand-left");
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

IMPLEMENT_FACTORY(Yeti, "yeti")
