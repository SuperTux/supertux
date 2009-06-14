//  $Id$
//
//  SuperTux -- Explosion object
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
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

#include "explosion.hpp"

#include "badguy/badguy.hpp"
#include "object/sprite_particle.hpp"
#include "random_generator.hpp"
#include "object_factory.hpp"
#include "audio/sound_manager.hpp"
#include "sector.hpp"

#include <math.h>

Explosion::Explosion(const Vector& pos)
        : MovingSprite(pos, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_TOUCHABLE), state(STATE_WAITING)
{
  sound_manager->preload("sounds/explosion.wav");
  set_pos(get_pos() - (get_bbox().get_middle() - get_pos()));
}

Explosion::Explosion(const lisp::Lisp& reader)
        : MovingSprite(reader, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_TOUCHABLE), state(STATE_WAITING)
{
  sound_manager->preload("sounds/explosion.wav");
}

void
Explosion::explode()
{
  if (state != STATE_WAITING) return;
  state = STATE_EXPLODING;

  set_action("default", 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action
  sound_manager->play("sounds/explosion.wav", get_pos());

  if (0)
    {
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
        Sector::current()->add_object(new SpriteParticle("images/objects/particles/explosion.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
      }
    }
}

void 
Explosion::update(float )
{
  switch(state) {
    case STATE_WAITING:
      explode();
      break;
    case STATE_EXPLODING:
      if(sprite->animation_done()) {
        remove_me();
      }
      break;
  }
}

HitResponse
Explosion::collision(GameObject& other, const CollisionHit& )
{
  if(state != STATE_EXPLODING) return ABORT_MOVE;

  Player* player = dynamic_cast<Player*>(&other);
  if(player != 0) {
    player->kill(false);
  }

  BadGuy* badguy = dynamic_cast<BadGuy*>(&other);
  if(badguy != 0) {
    badguy->kill_fall();
  }

  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Explosion, "explosion");

