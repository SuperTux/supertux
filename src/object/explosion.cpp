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
#include "math/random.hpp"
#include "object/particles.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

Explosion::Explosion(const Vector& pos, float p_push_strength,
    int p_num_particles) :
  MovingSprite(pos, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push_strength(p_push_strength),
  num_particles(p_num_particles),
  state(STATE_WAITING),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-large.sprite"))
{
  SoundManager::current()->preload("sounds/explosion.wav");
  SoundManager::current()->preload("sounds/firecracker.ogg");
  set_pos(get_pos() - (m_col.m_bbox.get_middle() - get_pos()));
  lightsprite->set_blend(Blend::ADD);
  lightsprite->set_color(Color(0.6f, 0.6f, 0.6f));
}

Explosion::Explosion(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/explosion/explosion.sprite", LAYER_OBJECTS+40, COLGROUP_MOVING),
  hurt(true),
  push_strength(-1),
  num_particles(100),
  state(STATE_WAITING),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-large.sprite"))
{
  SoundManager::current()->preload("sounds/explosion.wav");
  SoundManager::current()->preload("sounds/firecracker.ogg");
  lightsprite->set_blend(Blend::ADD);
  lightsprite->set_color(Color(0.6f, 0.6f, 0.6f));
}

void
Explosion::explode()
{
  if (state != STATE_WAITING)
    return;
  state = STATE_EXPLODING;

  set_action(hurt ? "default" : "pop", 1);
  m_sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action
  m_sprite->set_angle(graphicsRandom.randf(0, 360)); // a random rotation on the sprite to make explosions appear more random
  if (hurt)
    SoundManager::current()->play("sounds/explosion.wav", get_pos(), 0.98f);
  else
    SoundManager::current()->play("sounds/firecracker.ogg", get_pos(), 0.7f);
  bool does_push = push_strength > 0;

  // spawn some particles
  Vector accel = Vector(0, Sector::get().get_gravity()*100);
  Sector::get().add<Particles>(
    m_col.m_bbox.get_middle(), -360, 360, 450, 900, accel, num_particles,
    Color(.4f, .4f, .4f), 3, .8f, LAYER_OBJECTS-1);

  if (does_push) {
    Vector center = m_col.m_bbox.get_middle ();
    auto near_objects = Sector::get().get_nearby_objects (center, 128.0 * 32.0);

    for (auto& obj: near_objects) {
      Vector obj_vector = obj->get_bbox ().get_middle ();
      Vector direction = obj_vector - center;
      float distance = direction.norm ();

      /* If the distance is very small, for example because "obj" is the badguy
       * causing the explosion, skip this object. */
      if (distance <= 1.0f)
        continue;

      /* The force decreases with the distance squared. In the distance of one
       * tile (32 pixels) you will have a speed increase of 150 pixels/s. */
      float force = push_strength / (distance * distance);
      // If we somehow get a force of over 200, keep it at 200 because
      // unexpected behaviour could result otherwise.
      if (force > 200.0f)
        force = 200.0;

      Vector add_speed = direction.unit () * force;

      auto player = dynamic_cast<Player *> (obj);
      if (player) {
        player->add_velocity (add_speed);
      }

      auto badguy = dynamic_cast<WalkingBadguy *> (obj);
      if (badguy && badguy->is_active()) {
        badguy->add_velocity (add_speed);
      }
    }
  }
}

void
Explosion::update(float )
{
  switch (state) {
    case STATE_WAITING:
      explode();
      break;
    case STATE_EXPLODING:
      if (m_sprite->animation_done()) {
        remove_me();
      }
      break;
  }
}

void
Explosion::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS+40);
  lightsprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);
}

HitResponse
Explosion::collision(GameObject& other, const CollisionHit& )
{
  if ((state != STATE_EXPLODING) || !hurt || m_sprite->get_current_frame() > 8)
    return ABORT_MOVE;

  auto player = dynamic_cast<Player*>(&other);
  if (player != nullptr) {
    player->kill(false);
  }

  auto badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy != nullptr) {
    badguy->kill_fall();
  }

  return ABORT_MOVE;
}

/* EOF */
