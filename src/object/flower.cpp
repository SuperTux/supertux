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
#include "util/log.hpp"

Flower::Flower(PowerUp::Type _type, const std::string& custom_sprite) :
  m_type(_type),
  m_sprite(),
  m_flip(NO_FLIP),
  m_lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  m_col.m_bbox.set_size(32, 32);
  m_lightsprite->set_blend(Blend::ADD);

  switch (m_type) {
    case PowerUp::Type::FIRE:
    case PowerUp::Type::COFFEE:
    {
      m_bonus = BONUS_FIRE;
      m_sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/fireflower/fireflower.sprite" : custom_sprite);
      SoundManager::current()->preload("sounds/fire-flower.wav");
      m_lightsprite->set_color(Color(0.3f, 0.0f, 0.0f));
      break;
    }
    case PowerUp::Type::ICE:
    {
      m_bonus = BONUS_ICE;
      m_sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/iceflower/iceflower.sprite" : custom_sprite);
      SoundManager::current()->preload("sounds/fire-flower.wav");
      m_lightsprite->set_color(Color(0.0f, 0.1f, 0.2f));
      break;
    }
    case PowerUp::Type::AIR:
    {
      m_bonus = BONUS_AIR;
      m_sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/airflower/airflower.sprite" : custom_sprite);
      SoundManager::current()->preload("sounds/fire-flower.wav");
      m_lightsprite->set_color(Color(0.15f, 0.0f, 0.15f));
      break;
    }
    case PowerUp::Type::EARTH:
    {
      m_bonus = BONUS_EARTH;
      m_sprite = SpriteManager::current()->create(custom_sprite.empty() ? "images/powerups/earthflower/earthflower.sprite" : custom_sprite);
      SoundManager::current()->preload("sounds/fire-flower.wav");
      m_lightsprite->set_color(Color(0.0f, 0.3f, 0.0f));
      break;
    }
    default:
    {
      log_fatal << "Invalid flower type '" << m_type << "'" << std::endl;
      assert(false);
    }
  }

  set_group(COLGROUP_TOUCHABLE);
}

void
Flower::update(float )
{
}

void
Flower::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), LAYER_OBJECTS, m_flip);
  m_lightsprite->draw(context.light(), m_col.m_bbox.get_middle(), 0);
}

HitResponse
Flower::collision(MovingObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if (!player)
    return ABORT_MOVE;

  if (!player->add_bonus(m_bonus, true))
    return FORCE_MOVE;

  SoundManager::current()->play("sounds/fire-flower.wav", get_pos());
  remove_me();
  return ABORT_MOVE;
}

void
Flower::on_flip(float height)
{
  MovingObject::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}
