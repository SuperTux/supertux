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

#include "object/flower.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"

Flower::Flower(BonusType _type, const std::string& custom_sprite) :
  type(_type),
  sprite(),
  flip(NO_FLIP),
  lightsprite()
{
  m_col.m_bbox.set_size(32, 32);

  if (type == FIRE_BONUS) {
    sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/fireflower/fireflower.sprite" : custom_sprite);
    SoundManager::current()->preload("sounds/fire-flower.wav");
  }
  else if (type == ICE_BONUS) {
    sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/iceflower/iceflower.sprite" : custom_sprite);
    SoundManager::current()->preload("sounds/fire-flower.wav");
  }
  else if (type == AIR_BONUS) {
    sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/airflower/airflower.sprite" : custom_sprite);
    SoundManager::current()->preload("sounds/fire-flower.wav");
  }
  else if (type == EARTH_BONUS) {
    sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/earthflower/earthflower.sprite" : custom_sprite);
    SoundManager::current()->preload("sounds/fire-flower.wav");
  }
  else {
    assert(false);
  }

  lightsprite = sprite->get_linked_light_sprite();
  if (lightsprite)
    lightsprite->set_blend(Blend::ADD);

  set_group(COLGROUP_TOUCHABLE);
}

void
Flower::update(float )
{
}

void
Flower::draw(DrawingContext& context)
{
  sprite->draw(context.color(), get_pos(), LAYER_OBJECTS, flip);
  if (lightsprite)
    lightsprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);
}

HitResponse
Flower::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if (!player)
    return ABORT_MOVE;

  if (!player->add_bonus(type, true))
    return FORCE_MOVE;

  SoundManager::current()->play("sounds/fire-flower.wav", get_pos());
  remove_me();
  return ABORT_MOVE;
}

void
Flower::on_flip(float height)
{
  MovingObject::on_flip(height);
  FlipLevelTransformer::transform_flip(flip);
}

/* EOF */
