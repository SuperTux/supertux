//  $Id: igel.cpp 3478 2006-04-30 23:14:15Z sommer $
//
//  SuperTux - Badguy "Igel"
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

#include "igel.hpp"
#include "object/block.hpp"
#include "sector.hpp"
#include "object/bullet.hpp"

namespace {
  const float WALKSPEED = 80; /**< speed at which we walk around */
  const float TURN_RECOVER_TIME = 0.5; /**< seconds before we will again turn around when shot at */
  const float RANGE_OF_VISION = 256; /**< range in px at which we can see bullets */
}

Igel::Igel(const lisp::Lisp& reader)
  : BadGuy(reader, "images/creatures/igel/igel.sprite"), state(STATE_NORMAL)
{
  set_direction = false;
}

Igel::Igel(const Vector& pos, Direction d)
  : BadGuy(pos, "images/creatures/igel/igel.sprite"), state(STATE_NORMAL)
{
  set_direction = true;
  initial_direction = d;
}

void
Igel::write(lisp::Writer& writer)
{
  writer.start_list("igel");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);

  writer.end_list("igel");
}

void
Igel::activate()
{
  if (set_direction) {dir = initial_direction;}

  be_normal();
}

void
Igel::be_normal()
{
  state = STATE_NORMAL;
  sprite->set_action(dir == LEFT ? "walking-left" : "walking-right");

  physic.set_velocity_x(dir == LEFT ? -WALKSPEED : WALKSPEED);
}

void
Igel::turn_around()
{
  dir = (dir == LEFT ? RIGHT : LEFT);
  turn_recover_timer.start(TURN_RECOVER_TIME);
  be_normal();
}

bool
Igel::can_see(const MovingObject& o)
{
  Rect mb = get_bbox();
  Rect ob = o.get_bbox();

  bool inReach_left = (ob.p2.x >= mb.p1.x-((dir == LEFT) ? RANGE_OF_VISION : 0));
  bool inReach_right = (ob.p1.x <= mb.p2.x+((dir == RIGHT) ? RANGE_OF_VISION : 0));
  bool inReach_top = (ob.p2.y >= mb.p1.y);
  bool inReach_bottom = (ob.p1.y <= mb.p2.y);

  return (inReach_left && inReach_right && inReach_top && inReach_bottom);
}

void
Igel::active_update(float elapsed_time)
{
  switch (state) {

    case STATE_NORMAL:
      if (might_fall()) {
	// turn around when we are at a ledge
	turn_around();
      } 
      else if (!turn_recover_timer.started()) {
	// turn around when we see a Bullet
	Sector* sector = Sector::current();
	for (Sector::GameObjects::iterator i = sector->gameobjects.begin(); i != sector->gameobjects.end(); ++i) {
	  Bullet* bullet = dynamic_cast<Bullet*>(*i);
	  if (bullet) {
	    if (can_see(*bullet)) turn_around();
	  }
	}
      }
      break;

  }

  BadGuy::active_update(elapsed_time);
}

HitResponse
Igel::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // floor or roof
    physic.set_velocity_y(0);
    return CONTINUE;
  }

  // hit left or right
  switch(state) {

    case STATE_NORMAL:
      turn_around();
      break;

  }

  return CONTINUE;
}

HitResponse
Igel::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5) { // floor or roof
    physic.set_velocity_y(0);
    return CONTINUE;
  }

  // hit left or right
  switch(state) {

    case STATE_NORMAL:
      turn_around();
      break;

  }

  return CONTINUE;
}

HitResponse
Igel::collision_bullet(Bullet& , const CollisionHit& hit)
{
  // die if hit on front side
  if (((dir == LEFT) && (hit.normal.x > 0)) || ((dir == RIGHT) && (hit.normal.x < 0))) {
    kill_fall();
    return ABORT_MOVE;
  }

  // else ignore bullet
  return FORCE_MOVE;
}

bool
Igel::collision_squished(Player& )
{
  switch(state) {

    case STATE_NORMAL:
      // this will hurt
      return false;
      break;

  }

  kill_fall();
  return true;
}

IMPLEMENT_FACTORY(Igel, "igel")
