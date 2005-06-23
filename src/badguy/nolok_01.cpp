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

#include "nolok_01.h"
#include "badguy/bouncing_snowball.h"
#include "trigger/door.h"

#define WALK_TIME 2.5
#define SHOOT_TIME 0.4
#define JUMP_TIME 0.5
#define INITIAL_HITPOINTS 3
#define INITIAL_BULLET_HP 10

static const float WALKSPEED = 90;

//TODO: Create sprite, limit max number of snowballs
Nolok_01::Nolok_01(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  bbox.set_size(31.8, 63.8);
  sprite = sprite_manager->create("dummyguy");
  countMe = false;
}

Nolok_01::Nolok_01(float pos_x, float pos_y)
{
  start_position.x = pos_x;
  start_position.y = pos_y;
  bbox.set_size(31.8, 63.8);
  sprite = sprite_manager->create("dummyguy");
  countMe = false;
}

void
Nolok_01::write(lisp::Writer& writer)
{
  writer.start_list("nolok_01");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("nolok_01");
}

void
Nolok_01::activate()
{
  //hitpoints = INITIAL_HITPOINTS;
  //bullet_hitpoints = INITIAL_BULLET_HP;
  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
  sprite->set_action(dir == LEFT ? "left" : "right");
  action = WALKING;
  action_timer.start(WALK_TIME);
}

void
Nolok_01::active_update(float elapsed_time)
{
   if (action_timer.check()) {
     switch (action) {       
       case WALKING:
        {
         sprite->set_action("jump");
         physic.set_velocity_y(700);
         action = JUMPING;
         action_timer.start(JUMP_TIME);
         break;
        }
       case JUMPING:
       {
        sprite->set_action("throw");
        action = SHOOTING;
        action_timer.start(SHOOT_TIME);
        break;
       }
       case SHOOTING:
       {
        Sector::current()->add_object(new BouncingSnowball(get_pos().x - 64, get_pos().y, LEFT));
        Sector::current()->add_object(new BouncingSnowball(get_pos().x + 64, get_pos().y, RIGHT));
        physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
        sprite->set_action(dir == LEFT ? "left" : "right");
        action = WALKING;
        action_timer.start(WALK_TIME);
        break;
       }
     }
   }
   movement = physic.get_movement(elapsed_time);
}

bool
Nolok_01::collision_squished(Player& player)
{
  bool result = false;
  player.bounce(*this);
#if 0
  if (hitpoints <= 0) {
    bullet_hitpoints = 0;
    sprite->set_action("dead"); 
    kill_squished(player);
    Sector::current()->add_object(new Door((int)get_pos().x+32, 512, "sector1", "main2"));
    result = true;
  }
#endif
  return result;
}

HitResponse
Nolok_01::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5){ // hit floor or roof?
    if (action != JUMPING) physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

//TODO: Hitpoint count incorrect when combining squishing and shooting
void
Nolok_01::kill_fall()
{
#if 0
  bullet_hitpoints--;
  if (bullet_hitpoints <= 0) {
   hitpoints = 0;
   sound_manager->play("fall", this,
                             Sector::current()->player->get_pos());
   physic.set_velocity_y(0);
   physic.enable_gravity(true);
   set_state(STATE_FALLING);
   Sector::current()->add_object(new Door((int)get_pos().x+32, 512, "sector1", "main2"));
  }
#endif
}

IMPLEMENT_FACTORY(Nolok_01, "nolok_01")
