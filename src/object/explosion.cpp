//  SuperTux -- Explosion object
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2007.expires.deltadevelopment.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "object/explosion.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "badguy/walking_badguy.hpp"
#include "math/random_generator.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

Explosion::Explosion(const Vector& pos) :
  MovingSprite(pos, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push(false),
  state(STATE_WAITING)
{
  sound_manager->preload("sounds/explosion.wav");
  set_pos(get_pos() - (get_bbox().get_middle() - get_pos()));
}

Explosion::Explosion(const Reader& reader) :
  MovingSprite(reader, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push(false),
  state(STATE_WAITING)
{
  sound_manager->preload("sounds/explosion.wav");
}

void
Explosion::explode()
{
  if (state != STATE_WAITING)
    return;
  state = STATE_EXPLODING;

  set_action("default", 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action
  sound_manager->play("sounds/explosion.wav", get_pos());

#if 0
  // spawn some particles
  // TODO: provide convenience function in MovingSprite or MovingObject?
  for (int i = 0; i < 100; i++) {
    Vector ppos = bbox.get_middle();
    float angle = graphicsRandom.randf(-M_PI_2, M_PI_2);
    float velocity = graphicsRandom.randf(450, 900);
    float vx = sin(angle)*velocity;
    float vy = -cos(angle)*velocity;
    Vector pspeed = Vector(vx, vy);
    Vector paccel = Vector(0, 1000);
    Sector::current()->add_object(new SpriteParticle("images/objects/particles/explosion.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1));
  }
#endif

  if (push) {
    Vector center = get_bbox ().get_middle ();
    std::vector<MovingObject*> near_objects = Sector::current()->get_nearby_objects (center, 10.0 * 32.0);

    for (size_t i = 0; i < near_objects.size (); i++) {
      MovingObject *obj = near_objects[i];
      Vector obj_vector = obj->get_bbox ().get_middle ();
      Vector direction = obj_vector - center;
      float distance = direction.norm ();

      /* If the distance is very small, for example because "obj" is the badguy
       * causing the explosion, skip this object. */
      if (distance <= 1.0)
        continue;

      /* The force decreases with the distance squared. In the distance of one
       * tile (32 pixels) you will have a speed increase of 150 pixels/s. */
      float force = 150.0 * 32.0*32.0 / (distance * distance);
      if (force > 200.0)
        force = 200.0;

      Vector add_speed = direction.unit () * force;

      Player *player = dynamic_cast<Player *> (obj);
      if (player) {
        player->add_velocity (add_speed);
      }

      WalkingBadguy *badguy = dynamic_cast<WalkingBadguy *> (obj);
      if (badguy) {
        badguy->add_velocity (add_speed);
      }
    } /* for (i = 0 ... near_objects) */
  } /* if (push) */
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
  if ((state != STATE_EXPLODING) || !hurt)
    return ABORT_MOVE;

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

/* EOF */
