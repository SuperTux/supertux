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

#include "sspiky.hpp"

static const float WALKSPEED = 80;
static const float WAKE_TIME = .5;

SSpiky::SSpiky(const lisp::Lisp& reader)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  stay_on_platform = false;
  reader.get("stay-on-platform", stay_on_platform);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/spiky/sleepingspiky.sprite");
  state = SSPIKY_SLEEPING;
}

void
SSpiky::write(lisp::Writer& writer)
{
  writer.start_list("sspiky");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  if (stay_on_platform) writer.write_bool("stay-on-platform", true);

  writer.end_list("sspiky");
}

void
SSpiky::activate()
{
  //FIXME: turns sspiky around for debugging
  dir = dir == LEFT ? RIGHT : LEFT;

  state = SSPIKY_SLEEPING;
  physic.set_velocity_x(0);
  sprite->set_action(dir == LEFT ? "sleeping-left" : "sleeping-right");
}



HitResponse
SSpiky::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    physic.set_velocity_y(0);
  } else { // hit right or left
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

HitResponse
SSpiky::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(state != SSPIKY_WALKING) return CONTINUE;

  if(fabsf(hit.normal.x) > .8) { // left or right
    dir = dir == LEFT ? RIGHT : LEFT;
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }

  return CONTINUE;
}

void 
SSpiky::active_update(float elapsed_time) {
  BadGuy::active_update(elapsed_time);

  if(state == SSPIKY_SLEEPING) {

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
        state = SSPIKY_WAKING;
      }
    }
  }

  if(state == SSPIKY_WAKING) {
    if(timer.check()) {
      // start walking
      sprite->set_action(dir == LEFT ? "left" : "right");
      physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
      state = SSPIKY_WALKING;
    }
  }

  if (state == SSPIKY_WALKING && stay_on_platform && may_fall_off_platform())
  {
    dir = (dir == LEFT ? RIGHT : LEFT);
    sprite->set_action(dir == LEFT ? "left" : "right");
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

IMPLEMENT_FACTORY(SSpiky, "sspiky")
