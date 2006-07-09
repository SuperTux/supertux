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
#include "object/sprite_particle.hpp"

Bomb::Bomb(const Vector& pos, Direction dir, std::string custom_sprite /*= "images/creatures/mr_cherry/cherry.sprite"*/ )
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
Bomb::collision_player(Player& player, const CollisionHit& )
{
  if(state == STATE_EXPLODING) {
    player.kill(false);
  }
  return ABORT_MOVE;
}

HitResponse
Bomb::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  if(state == STATE_EXPLODING)
    badguy.kill_fall();
  return ABORT_MOVE;
}

void
Bomb::active_update(float )
{
  switch(state) {
    case STATE_TICKING:
      ticking->set_position(get_pos());
      if(sprite->animation_done()) {
        explode();
      }
      break;
    case STATE_EXPLODING:
      if(sprite->animation_done()) {
        remove_me();
      }
      break;
  } 
}

void
Bomb::explode()
{
  ticking->stop();
  state = STATE_EXPLODING;
  set_group(COLGROUP_TOUCHABLE);
  sound_manager->play("sounds/explosion.wav", get_pos());
  set_action_centered("explosion", 1);

  // spawn some particles
  // TODO: provide convenience function in MovingSprite or MovingObject?
  for (int i = 0; i < 100; i++) {
    Vector ppos = bbox.get_middle();
    float angle = systemRandom.randf(-M_PI_2, M_PI_2);
    float velocity = systemRandom.randf(450, 900);
    float vx = sin(angle)*velocity;
    float vy = -cos(angle)*velocity;
    Vector pspeed = Vector(vx, vy);
    Vector paccel = Vector(0, 1000);
    Sector::current()->add_object(new SpriteParticle("images/objects/particles/kracker.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
  }

}

void
Bomb::kill_fall()
{
  if (state != STATE_EXPLODING)  // we don't want it exploding again
    explode();
}

