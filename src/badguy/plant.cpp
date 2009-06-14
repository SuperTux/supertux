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

#include "plant.hpp"

#include "lisp/writer.hpp"
#include "object_factory.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const float WALKSPEED = 80;
static const float WAKE_TIME = .5;

Plant::Plant(const lisp::Lisp& reader)
        : BadGuy(reader, "images/creatures/plant/plant.sprite")
{
  state = PLANT_SLEEPING;
}

void
Plant::write(lisp::Writer& writer)
{
  writer.start_list("plant");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);

  writer.end_list("plant");
}

void
Plant::initialize()
{
  //FIXME: turns sspiky around for debugging
  dir = dir == LEFT ? RIGHT : LEFT;

  state = PLANT_SLEEPING;
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}

void
Plant::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
Plant::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(state != PLANT_WALKING) return CONTINUE;

  if(hit.left || hit.right) {
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

void
Plant::active_update(float elapsed_time) {
  BadGuy::active_update(elapsed_time);

  if(state == PLANT_SLEEPING) {

    Player* player = this->get_nearest_player();
    if (player) {
      Rect mb = this->get_bbox();
      Rect pb = player->get_bbox();

      bool inReach_left = (pb.p2.x >= mb.p2.x-((dir == LEFT) ? 256 : 0));
      bool inReach_right = (pb.p1.x <= mb.p1.x+((dir == RIGHT) ? 256 : 0));
      bool inReach_top = (pb.p2.y >= mb.p2.y);
      bool inReach_bottom = (pb.p1.y <= mb.p1.y);

      if (inReach_left && inReach_right && inReach_top && inReach_bottom) {
        // wake up
        sprite->set_action(dir == LEFT ? "waking-left" : "waking-right");
        if(!timer.started()) timer.start(WAKE_TIME);
        state = PLANT_WAKING;
      }
    }
  }

  if(state == PLANT_WAKING) {
    if(timer.check()) {
      // start walking
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
      state = PLANT_WALKING;
    }
  }

}

IMPLEMENT_FACTORY(Plant, "plant")
