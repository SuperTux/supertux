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

#include <config.h>

#include "badguy/kugelblitz.hpp"
#include "object/tilemap.hpp"
#include "object/camera.hpp"
#include "supertux/tile.hpp"
#include "math/random_generator.hpp"
#include "lisp/writer.hpp"
#include "supertux/object_factory.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "sprite/sprite.hpp"

#define  LIFETIME 5
#define  MOVETIME 0.75
#define  BASE_SPEED 200
#define  RAND_SPEED 150

static const float X_OFFSCREEN_DISTANCE = 1600;
static const float Y_OFFSCREEN_DISTANCE = 1200;

Kugelblitz::Kugelblitz(const lisp::Lisp& reader)
    : BadGuy(reader, "images/creatures/kugelblitz/kugelblitz.sprite"), groundhit_pos_set(false)
{
  reader.get("x", start_position.x);
  sprite->set_action("falling");
  physic.enable_gravity(false);
  countMe = false;
}

void
Kugelblitz::write(lisp::Writer& writer)
{
  writer.start_list("kugelblitz");

  writer.write("x", start_position.x);

  writer.end_list("kugelblitz");
}

void
Kugelblitz::initialize()
{
  physic.set_velocity_y(300);
  physic.set_velocity_x(-20); //fall a little to the left
  direction = 1;
  dying = false;
}

void
Kugelblitz::collision_solid(const CollisionHit& chit)
{
  hit(chit);
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
      player.kill(false);
      explode();
    return FORCE_MOVE;
  }
  player.kill(false);
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
Kugelblitz::hit(const CollisionHit& hit)
{
  // hit floor?
  if(hit.bottom) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }
    sprite->set_action("flying");
    physic.set_velocity_y(0);
    //Set random initial speed and direction
    direction = systemRandom.rand(2)? 1: -1;
    int speed = (BASE_SPEED + (systemRandom.rand(RAND_SPEED))) * direction;
    physic.set_velocity_x(speed);
    movement_timer.start(MOVETIME);
    lifetime.start(LIFETIME);

  } else if(hit.top) { // bumped on roof
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
        int speed = (BASE_SPEED + (systemRandom.rand(RAND_SPEED))) * direction;
        physic.set_velocity_x(speed);
        movement_timer.start(MOVETIME);
      }
    }
    /*
    if (Sector::current()->solids->get_tile_at(get_pos())->getAttributes() == 16) {
      //HIT WATER
      Sector::current()->add_object(new Electrifier(75,1421,1.5));
      Sector::current()->add_object(new Electrifier(76,1422,1.5));
      explode();
    }
    if (Sector::current()->solids->get_tile_at(get_pos())->getAttributes() == 48) {
      //HIT ELECTRIFIED WATER
      explode();
    }
    */
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
    lifetime.start(0.2f);
    dying = true;
  }
  else remove_me();
}

void
Kugelblitz::try_activate()
{
  //FIXME: Don't activate Kugelblitz before it's on-screen
  float scroll_x = Sector::current()->camera->get_translation().x;
  float scroll_y = Sector::current()->camera->get_translation().y;

  /* Activate badguys if they're just around the screen to avoid
   * the effect of having badguys suddenly popping up from nowhere.
   */
  if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x - bbox.get_width() &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = RIGHT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
      start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if (start_position.x > scroll_x - X_OFFSCREEN_DISTANCE &&
      start_position.x < scroll_x + X_OFFSCREEN_DISTANCE &&
      ((start_position.y > scroll_y &&
        start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) ||
       (start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE &&
        start_position.y < scroll_y))) {
    dir = start_position.x < scroll_x ? RIGHT : LEFT;
    set_state(STATE_ACTIVE);
    activate();
  } else if(state == STATE_INIT
      && start_position.x > scroll_x - X_OFFSCREEN_DISTANCE
      && start_position.x < scroll_x + X_OFFSCREEN_DISTANCE
      && start_position.y > scroll_y - Y_OFFSCREEN_DISTANCE
      && start_position.y < scroll_y + Y_OFFSCREEN_DISTANCE) {
    dir = LEFT;
    set_state(STATE_ACTIVE);
    activate();
  }
}

IMPLEMENT_FACTORY(Kugelblitz, "kugelblitz");
