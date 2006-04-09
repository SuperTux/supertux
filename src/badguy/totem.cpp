//  $Id: totem.cpp 3096 2006-03-17 12:03:02Z sommer $
// 
//  SuperTux - "Totem" Badguy
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <config.h>

#include "totem.hpp"
#include "msg.hpp"

static const float WALKSPEED = 100;
static const float JUMP_ON_SPEED_Y = 400;
static const float JUMP_OFF_SPEED_Y = 500;

Totem::Totem(const lisp::Lisp& reader)
{
  stay_on_platform = false;
  carrying = 0;
  carried_by = 0;
  bbox.set_size(48, 49);

  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  reader.get("stay-on-platform", stay_on_platform);
  sprite = sprite_manager->create("images/creatures/totem/totem.sprite");
}

Totem::~Totem() 
{
  if (carrying) carrying->jump_off();
  if (carried_by) jump_off();
}

void
Totem::write(lisp::Writer& writer)
{
  writer.start_list("totem");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_bool("stay-on-platform", stay_on_platform);

  writer.end_list("totem");
}

void
Totem::activate()
{
  if (!carried_by) {
    physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
    sprite->set_action(dir == LEFT ? "walking-left" : "walking-right");
    return;
  } else {
    synchronize_with(carried_by);
    sprite->set_action(dir == LEFT ? "stacked-left" : "stacked-right");
    return;
  }
}

void
Totem::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if (!carried_by) {
    if (stay_on_platform && may_fall_off_platform())
    {
      dir = (dir == LEFT ? RIGHT : LEFT);
      activate();
    }

    Sector* s = Sector::current();
    if (s) {
      // jump a bit if we find a suitable totem 
      for (std::vector<MovingObject*>::iterator i = s->moving_objects.begin(); i != s->moving_objects.end(); i++) {
	Totem* t = dynamic_cast<Totem*>(*i);
	if (!t) continue;
	
	// skip if we are not approaching each other
	if (!((this->dir == LEFT) && (t->dir == RIGHT))) continue;
	
	Vector p1 = this->get_pos();
	Vector p2 = t->get_pos();

	// skip if not on same height
	float dy = (p1.y - p2.y);
	if (fabsf(dy - 0) > 2) continue;

	// skip if too far away
	float dx = (p1.x - p2.x);
	if (fabsf(dx - 128) > 2) continue;

	physic.set_velocity_y(JUMP_ON_SPEED_Y);
	p1.y -= 1;
	this->set_pos(p1);
	break;
      }
    }
  }

  if (carried_by) {
    this->synchronize_with(carried_by);
  }

  if (carrying) {
    carrying->synchronize_with(this);
  }

}

bool
Totem::collision_squished(Player& player)
{
  if (carrying) carrying->jump_off();
  if (carried_by) {
    player.bounce(*this);
    jump_off();
  }

  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  this->bbox.set_size(48, 45);
  kill_squished(player);
  return true;
}

HitResponse
Totem::collision_solid(GameObject& object, const CollisionHit& hit)
{
  // if we are being carried around, pass event to bottom of stack and ignore it
  if (carried_by) {
    carried_by->collision_solid(object, hit);
    return CONTINUE;
  }

  // If we hit something from above or below: stop moving in this direction 
  if (hit.normal.y != 0) {
    physic.set_velocity_y(0);
  }

  // If we are hit from the direction we are facing: turn around
  if ((hit.normal.x > .8) && (dir == LEFT)) {
    dir = RIGHT;
    activate();
  }
  if ((hit.normal.x < -.8) && (dir == RIGHT)) {
    dir = LEFT;
    activate();
  }

  return CONTINUE;
}

HitResponse
Totem::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  // if we are being carried around, pass event to bottom of stack and ignore it
  if (carried_by) {
    carried_by->collision_badguy(badguy, hit);
    return CONTINUE;
  }
 
  // if we hit a Totem that is not from our stack: have our base jump on its top
  Totem* totem = dynamic_cast<Totem*>(&badguy);
  if (totem) {
    Totem* thisBase = this; while (thisBase->carried_by) thisBase=thisBase->carried_by;
    Totem* srcBase = totem; while (srcBase->carried_by)  srcBase=srcBase->carried_by;
    Totem* thisTop = this;  while (thisTop->carrying)    thisTop=thisTop->carrying;
    if (srcBase != thisBase) {
      srcBase->jump_on(thisTop);
    }
  }

  // If we are hit from the direction we are facing: turn around
  if ((hit.normal.x > .8) && (dir == LEFT)) {
    dir = RIGHT;
    activate();
  }
  if ((hit.normal.x < -.8) && (dir == RIGHT)) {
    dir = LEFT;
    activate();
  }

  return CONTINUE;
}

void
Totem::kill_fall()
{
  if (carrying) carrying->jump_off();
  if (carried_by) jump_off();

  BadGuy::kill_fall();
}

void 
Totem::jump_on(Totem* target)
{
  if (target->carrying) {
    msg_warning << "target is already carrying someone" << std::endl;
    return;
  }
  
  target->carrying = this;

  this->carried_by = target;
  this->bbox.set_size(48, 45);
  this->activate();
  
  this->synchronize_with(target);
}

void
Totem::jump_off() {
  if (!carried_by) {
    msg_warning << "not carried by anyone" << std::endl;
    return;
  }

  carried_by->carrying = 0;

  this->carried_by = 0;
  this->bbox.set_size(48, 49);

  this->activate();

  physic.set_velocity_y(JUMP_OFF_SPEED_Y);
}

void 
Totem::synchronize_with(Totem* base)
{

  if (dir != base->dir) {
    dir = base->dir;
    sprite->set_action(dir == LEFT ? "stacked-left" : "stacked-right");
  }
  
  Vector pos = base->get_pos();
  pos.y -= 45;
  set_pos(pos);

  physic.set_velocity_x(base->physic.get_velocity_x());
  physic.set_velocity_y(base->physic.get_velocity_y());
}


IMPLEMENT_FACTORY(Totem, "totem")
  
