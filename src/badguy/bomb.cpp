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

#include "bomb.hpp"
#include "random_generator.hpp"
#include "object/explosion.hpp"

Bomb::Bomb(const Vector& pos, Direction dir, std::string custom_sprite /*= "images/creatures/mr_bomb/mr_bomb.sprite"*/ )
	: BadGuy( pos, dir, custom_sprite )
{
  state = STATE_TICKING;
  set_action(dir == LEFT ? "ticking-left" : "ticking-right", 1);
  countMe = false;

  ticking.reset(sound_manager->create_sound_source("sounds/fizz.wav"));
  ticking->set_position(get_pos());
  ticking->set_looping(true);
  ticking->set_gain(2.0);
  ticking->set_reference_distance(32);
  ticking->play();
}

Bomb::Bomb(const Bomb& other)
	: BadGuy(other), state(other.state)
{
  if (state == STATE_TICKING) {
    ticking.reset(sound_manager->create_sound_source("sounds/fizz.wav"));
    ticking->set_position(get_pos());
    ticking->set_looping(true);
    ticking->set_gain(2.0);
    ticking->set_reference_distance(32);
    ticking->play();
  }
}

void
Bomb::write(lisp::Writer& )
{
  // bombs are only temporarily so don't write them out...
}

void
Bomb::collision_solid(const CollisionHit& hit)
{
  if(hit.bottom)
    physic.set_velocity_y(0);
}

HitResponse
Bomb::collision_player(Player& , const CollisionHit& )
{
  return ABORT_MOVE;
}

HitResponse
Bomb::collision_badguy(BadGuy& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
Bomb::active_update(float )
{
  ticking->set_position(get_pos());
  if(sprite->animation_done()) {
    explode();
  }
}

void
Bomb::explode()
{
  ticking->stop();

  remove_me();
  Explosion* explosion = new Explosion(get_bbox().get_middle());
  Sector::current()->add_object(explosion);

  run_dead_script();
}

void
Bomb::kill_fall()
{
  explode();
}
