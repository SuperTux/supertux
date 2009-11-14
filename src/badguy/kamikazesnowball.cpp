//  $Id$
//
//  SuperTux
//  Copyright (C) 2008 Wolfgang Becker <uafr@gmx.de>
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

#include "kamikazesnowball.hpp"

#include "object_factory.hpp"
#include "audio/sound_manager.hpp"
#include "sprite/sprite.hpp"

/* 
 * Kamikaze Snowball will fly in one direction until he hits something.
 * On impact he is destroyed, trying to kill what he hit or hit him. 
 */
namespace{
  static const float SPEED = 200;
  const std::string SPLAT_SOUND = "sounds/splat.wav";
}

KamikazeSnowball::KamikazeSnowball(const lisp::Lisp& reader)
  : BadGuy(reader, "images/creatures/snowball/kamikaze-snowball.sprite")
{
  sound_manager->preload(SPLAT_SOUND);
}

KamikazeSnowball::KamikazeSnowball(const Vector& pos, Direction d)
  : BadGuy(pos, d, "images/creatures/snowball/kamikaze-snowball.sprite")
{
  sound_manager->preload(SPLAT_SOUND);
}

void
KamikazeSnowball::initialize()
{
  physic.set_velocity_x(dir == LEFT ? -SPEED : SPEED);
  physic.enable_gravity(false);
  sprite->set_action(dir == LEFT ? "left" : "right");
}

bool
KamikazeSnowball::collision_squished(GameObject& object)
{
  sprite->set_action(dir == LEFT ? "squished-left" : "squished-right");
  kill_squished(object);
  return true;
}

void
KamikazeSnowball::collision_solid(const CollisionHit& hit)
{
  if(hit.top || hit.bottom) {
    physic.set_velocity_y(0);
  } else if(hit.left || hit.right) {
    kill_collision();
  }
}

void
KamikazeSnowball::kill_collision()
{
    sprite->set_action(dir == LEFT ? "collision-left" : "collision-right");
    sound_manager->play(SPLAT_SOUND, get_pos());
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);
    physic.enable_gravity(true);
    set_state(STATE_FALLING);

    run_dead_script();
}

HitResponse
KamikazeSnowball::collision_player(Player& player, const CollisionHit& hit)
{
  HitResponse response = BadGuy::collision_player(player, hit);
  if(response == FORCE_MOVE){
    kill_collision();
    response = ABORT_MOVE;
  }
  return response;
}


IMPLEMENT_FACTORY(KamikazeSnowball, "kamikazesnowball")
