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
#include "object/particles.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

Explosion::Explosion(const Vector& pos) :
  MovingSprite(pos, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push(false),
  state(STATE_WAITING),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-large.sprite"))
{
  SoundManager::current()->preload("sounds/explosion.wav");
  SoundManager::current()->preload("sounds/firecracker.ogg");
  set_pos(get_pos() - (bbox.get_middle() - get_pos()));
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.6f, 0.6f, 0.6f));
}

Explosion::Explosion(const Reader& reader) :
  MovingSprite(reader, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push(false),
  state(STATE_WAITING),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-large.sprite"))
{
  SoundManager::current()->preload("sounds/explosion.wav");
  SoundManager::current()->preload("sounds/firecracker.ogg");
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.6f, 0.6f, 0.6f));
}

void
Explosion::explode()
{
  if (state != STATE_WAITING)
    return;
  state = STATE_EXPLODING;

  set_action(hurt ? "default" : "pop", 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action
  sprite->set_angle(graphicsRandom.randf(0, 360)); // a random rotation on the sprite to make explosions appear more random
  SoundManager::current()->play(hurt ? "sounds/explosion.wav" : "sounds/firecracker.ogg", get_pos());

  // spawn some particles
  int pnumber = push ? 8 : 100;
  Vector accel = Vector(0, Sector::current()->get_gravity()*100);
  Sector::current()->add_object(std::make_shared<Particles>(
    bbox.get_middle(), -360, 360, 450, 900, accel , pnumber, Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS-1));

  if (push) {
    Vector center = bbox.get_middle ();
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
      if (badguy && badguy->is_active()) {
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

void
Explosion::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS+40);
  //Explosions produce light (if ambient light is not maxed)
  context.get_light( bbox.get_middle(), &light);
  if (light.red + light.green + light.blue < 3.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, bbox.get_middle(), 0);
    context.pop_target();
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
