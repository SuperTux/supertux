//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "object/growup.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"

GrowUp::GrowUp(const Vector& pos, Direction direction) :
  MovingSprite(pos, "images/powerups/egg/egg.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  shadesprite(SpriteManager::current()->create("images/powerups/egg/egg.sprite")),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  physic.enable_gravity(true);
  physic.set_velocity_x((direction == Direction::LEFT) ? -100.0f : 100.0f);
  SoundManager::current()->preload("sounds/grow.ogg");
  //shadow to remain in place as egg rolls
  shadesprite->set_action("shadow");
  //set light for glow effect
  lightsprite->set_blend(Blend::ADD);
  lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
}

void
GrowUp::update(float dt_sec)
{
  m_col.set_movement(physic.get_movement(dt_sec));
}

void
GrowUp::draw(DrawingContext& context)
{
  if (physic.get_velocity_x() != 0) {
    m_sprite->set_angle(get_pos().x * 360.0f / (32.0f * math::PI));
  }
  MovingSprite::draw(context);
  shadesprite->draw(context.color(), get_pos(), m_layer);
  lightsprite->draw(context.light(), get_bbox().get_middle(), 0);
}

void
GrowUp::collision_solid(const CollisionHit& hit)
{
  if (hit.top)
    physic.set_velocity_y(0);
  if (hit.bottom && physic.get_velocity_y() > 0)
    physic.set_velocity_y(0);
  if (hit.left || hit.right) {
    physic.set_velocity_x(-physic.get_velocity_x());
  }
}

HitResponse
GrowUp::collision(GameObject& other, const CollisionHit& hit )
{
  auto player = dynamic_cast<Player*>(&other);
  if (player != nullptr) {
    if (!player->add_bonus(GROWUP_BONUS, true)) {
      // Tux can't grow right now.
      collision_solid( hit );
      return ABORT_MOVE;
    }

    SoundManager::current()->play("sounds/grow.ogg");
    remove_me();

    return ABORT_MOVE;
  }

  return FORCE_MOVE;
}

void
GrowUp::do_jump()
{
  physic.set_velocity_y(-300);
}

/* EOF */
