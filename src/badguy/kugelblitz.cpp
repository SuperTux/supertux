//  $Id: Kugelblitz.cpp 2654 2005-06-29 14:16:22Z wansti $
// 
//  SuperTux
//  Copyright (C) 2005 Marek Moeckel <wansti@gmx.de>
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

#include "kugelblitz.hpp"
#include "object/tilemap.hpp"
#include "tile.hpp"

#define  LIFETIME 5
#define  MOVETIME 0.75
#define  BASE_SPEED 200
#define  RAND_SPEED 150

Kugelblitz::Kugelblitz(const lisp::Lisp& reader)
    : groundhit_pos_set(false)
{
  reader.get("x", start_position.x);
  start_position.y = 0; //place above visible area
  bbox.set_size(63.8, 63.8);
  sprite = sprite_manager->create("kugelblitz");
  sprite->set_action("falling");
  physic.enable_gravity(false);
}

void
Kugelblitz::write(lisp::Writer& writer)
{
  writer.start_list("kugelblitz");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("kugelblitz");
}

void
Kugelblitz::activate()
{
  physic.set_velocity_y(-300);
  physic.set_velocity_x(-20); //fall a little to the left
  direction = 1;
  dying = false;
}

HitResponse
Kugelblitz::collision_solid(GameObject& , const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Kugelblitz::collision_player(Player& player, const CollisionHit& )
{
  if(player.is_invincible()) {
    explode();
    return ABORT_MOVE;
  }
  // hit from above?
  if(player.get_movement().y - get_movement().y > 0 && player.get_bbox().p2.y <
      (get_bbox().p1.y + get_bbox().p2.y) / 2) {
    // if it's not is it possible to squish us, then this will hurt
    if(!collision_squished(player))
      player.kill(Player::SHRINK);
      explode();
    return FORCE_MOVE;
  }
  player.kill(Player::SHRINK);
  explode();
  return FORCE_MOVE;
}

HitResponse
Kugelblitz::collision_badguy(BadGuy& other , const CollisionHit& chit)
{
  //Let the Kugelblitz explode, too? The problem with that is that
  //two Kugelblitzes would cancel each other out on contact...
  other.kill_fall();
  return hit(chit);
}

HitResponse
Kugelblitz::hit(const CollisionHit& chit)
{
  // hit floor?
  if(chit.normal.y < -.5) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }
    sprite->set_action("flying");
    physic.set_velocity_y(0);
    //Set random initial speed and direction
    if ((rand() % 2) == 1) direction = 1; else direction = -1;
    int speed = (BASE_SPEED + (rand() % RAND_SPEED)) * direction;
    physic.set_velocity_x(speed);
    movement_timer.start(MOVETIME);
    lifetime.start(LIFETIME);

  } else if(chit.normal.y < .5) { // bumped on roof
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Kugelblitz::active_update(float elapsed_time)
{
  if (lifetime.check()) {
    explode();
  }
  else {
    if (groundhit_pos_set) {
      if (movement_timer.check()) {
        if (direction == 1) direction = -1; else direction = 1;
        int speed = (BASE_SPEED + (rand() % RAND_SPEED)) * direction;
        physic.set_velocity_x(speed);
        movement_timer.start(MOVETIME);
      }
    }
    if (Sector::current()->solids->get_tile_at(get_pos())->getAttributes() == 16) {
      //HIT WATER
      Sector::current()->add_object(new Electrifier(75,1421,1.5));
      Sector::current()->add_object(new Electrifier(76,1422,1.5));
      explode();
    }
  }
  BadGuy::active_update(elapsed_time);  
}

void
Kugelblitz::kill_fall()
{
}

void
Kugelblitz::explode()
{
  if (!dying) {
    sprite->set_action("pop");
    lifetime.start(0.2);
    dying = true;
  }
  else remove_me();
}

IMPLEMENT_FACTORY(Kugelblitz, "kugelblitz")
