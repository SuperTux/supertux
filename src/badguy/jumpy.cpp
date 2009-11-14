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

#include "jumpy.hpp"

#include "lisp/writer.hpp"
#include "sprite/sprite.hpp"
#include "object_factory.hpp"
#include "object/player.hpp"

static const float JUMPSPEED=-600;
static const float JUMPY_MID_TOLERANCE=4;
static const float JUMPY_LOW_TOLERANCE=2;

Jumpy::Jumpy(const lisp::Lisp& reader)
    : BadGuy(reader, "images/creatures/snowjumpy/snowjumpy.sprite"), groundhit_pos_set(false)
{
  // TODO create a nice sound for this...
  //sound_manager->preload("sounds/skid.wav");
}

void
Jumpy::write(lisp::Writer& writer)
{
  writer.start_list("jumpy");

  writer.write("x", start_position.x);
  writer.write("y", start_position.y);

  writer.end_list("jumpy");
}

void
Jumpy::collision_solid(const CollisionHit& chit)
{
  hit(chit);
}

HitResponse
Jumpy::collision_badguy(BadGuy& , const CollisionHit& chit)
{
  return hit(chit);
}

HitResponse
Jumpy::hit(const CollisionHit& chit)
{
  if(chit.bottom) {
    if (!groundhit_pos_set)
    {
      pos_groundhit = get_pos();
      groundhit_pos_set = true;
    }

    physic.set_velocity_y((frozen || get_state() == STATE_FALLING) ? 0 : JUMPSPEED);
    // TODO create a nice sound for this...
    //sound_manager->play("sounds/skid.wav");
  } else if(chit.top) {
    physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Jumpy::active_update(float elapsed_time)
{
  BadGuy::active_update(elapsed_time);

  if(frozen)
    return;

  Player* player = this->get_nearest_player();
  if (player)
  {
    dir = (player->get_pos().x > get_pos().x) ? RIGHT : LEFT;
  }

  if (!groundhit_pos_set)
  {
    sprite->set_action(dir == LEFT ? "left-middle" : "right-middle");
    return;
  }

  if ( get_pos().y < (pos_groundhit.y - JUMPY_MID_TOLERANCE ) )
    sprite->set_action(dir == LEFT ? "left-up" : "right-up");
  else if ( get_pos().y >= (pos_groundhit.y - JUMPY_MID_TOLERANCE) &&
      get_pos().y < (pos_groundhit.y - JUMPY_LOW_TOLERANCE) )
    sprite->set_action(dir == LEFT ? "left-middle" : "right-middle");
  else
    sprite->set_action(dir == LEFT ? "left-down" : "right-down");
}

void
Jumpy::freeze()
{
  BadGuy::freeze();
  physic.set_velocity_y(std::max(0.0f, physic.get_velocity_y()));
  sprite->set_action(dir == LEFT ? "left-iced" : "right-iced");
}

bool
Jumpy::is_freezable() const
{
  return true;
}

IMPLEMENT_FACTORY(Jumpy, "jumpy")
