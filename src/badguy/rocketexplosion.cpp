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

#include "rocketexplosion.hpp"

static const float EXPLOSIONTIME = 1;

RocketExplosion::RocketExplosion(const Vector& pos, Direction dir)
{
  start_position = pos;
  bbox.set_pos(pos);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/mr_rocket/explosion.sprite");
  this->dir = dir;
  countMe = false;
  explode();
}

void
RocketExplosion::write(lisp::Writer& )
{
  // rocket explosions are only temporarily so don't write them out...
}

HitResponse
RocketExplosion::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(fabsf(hit.normal.y) > .5)
    physic.set_velocity_y(0);

  return CONTINUE;
}

HitResponse
RocketExplosion::collision_player(Player& player, const CollisionHit& )
{
  player.kill(Player::SHRINK);
  return ABORT_MOVE;
}

HitResponse
RocketExplosion::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
   badguy.kill_fall();
   return ABORT_MOVE;
}

void
RocketExplosion::active_update(float )
{
   if(timer.check()) {
      remove_me();
   }
}

void
RocketExplosion::explode()
{
  sprite->set_action(dir == LEFT ? "explosion-left" : "explosion-right");
  sound_manager->play("sounds/explosion.wav", get_pos());
  timer.start(EXPLOSIONTIME, true);
}

void
RocketExplosion::kill_fall()
{
}

