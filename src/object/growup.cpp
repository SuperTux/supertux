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

GrowUp::GrowUp(const Vector& pos, Direction direction, const std::string& custom_sprite) :
  MovingSprite(pos, custom_sprite.empty() ? "images/powerups/egg/egg.sprite" : custom_sprite, LAYER_OBJECTS, COLGROUP_MOVING),
  physic(),
  m_custom_sprite(!custom_sprite.empty()),
  shadesprite(m_sprite->get_linked_sprite("shade")),
  lightsprite(m_sprite->get_linked_sprite("light"))
{
  physic.enable_gravity(true);
  physic.set_velocity_x((direction == Direction::LEFT) ? -100.0f : 100.0f);
  SoundManager::current()->preload("sounds/grow.ogg");
  // Set the shadow action for the egg sprite, so it remains in place as the egg rolls.
  shadesprite->set_action("shadow");
  // Configure the light sprite for the glow effect.
  lightsprite->set_blend(Blend::ADD);
  lightsprite->set_color(Color(0.2f, 0.2f, 0.0f));
}

std::vector<MovingSprite::LinkedSprite>
GrowUp::get_linked_sprites()
{
  return {
    { "shade", shadesprite },
    { "light", lightsprite }
  };
}

void
GrowUp::update(float dt_sec)
{
  if (!m_custom_sprite && physic.get_velocity_x() != 0)
    m_sprite->set_angle(get_pos().x * 360.0f / (32.0f * math::PI));

  m_col.set_movement(physic.get_movement(dt_sec));
}

void
GrowUp::draw(DrawingContext& context)
{
  MovingSprite::draw(context);

  if (m_custom_sprite)
    return;

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

    SoundManager::current()->play("sounds/grow.ogg", get_pos());
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
