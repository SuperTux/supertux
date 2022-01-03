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

#include "object/firefly.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/game_session.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const Color TORCH_LIGHT_COLOR = Color(0.87f, 0.64f, 0.12f); /** Color of the light specific to the torch firefly sprite */
static const Vector TORCH_LIGHT_OFFSET = Vector(0, 12); /** Offset of the light specific to the torch firefly sprite */

Firefly::Firefly(const ReaderMapping& mapping) :
   MovingSprite(mapping, "images/objects/resetpoints/default-resetpoint.sprite", LAYER_TILES, COLGROUP_TOUCHABLE),
   m_sprite_light(),
   activated(false),
   initial_position(get_pos())
{
  if (!mapping.get( "sprite", m_sprite_name)){
    reactivate();
    return;
  }
  if (m_sprite_name.empty()) {
    m_sprite_name = "images/objects/resetpoints/default-resetpoint.sprite";
    reactivate();
    return;
  }
  //Replace sprite
  m_sprite = SpriteManager::current()->create( m_sprite_name );
  m_col.m_bbox.set_size(m_sprite->get_current_hitbox_width(), m_sprite->get_current_hitbox_height());

  if (m_sprite_name.find("torch", 0) != std::string::npos) {
    m_sprite_light = SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite");
    m_sprite_light->set_blend(Blend::ADD);
    m_sprite_light->set_color(TORCH_LIGHT_COLOR);
  }

  reactivate();

  //Load sound
  if ( m_sprite_name.find("vbell", 0) != std::string::npos ) {
    SoundManager::current()->preload("sounds/savebell_low.wav");
  }
  else if ( m_sprite_name.find("torch", 0) != std::string::npos ) {
    SoundManager::current()->preload("sounds/fire.ogg");
  }
  else {
    SoundManager::current()->preload("sounds/savebell2.wav");
  }
}

void
Firefly::draw(DrawingContext& context)
{
  MovingSprite::draw(context);

  if (m_sprite_name.find("torch", 0) != std::string::npos && (activated ||
        m_sprite->get_action() == "ringing")) {
    m_sprite_light->draw(context.light(), m_col.m_bbox.get_middle() + (m_flip == NO_FLIP ? -TORCH_LIGHT_OFFSET : TORCH_LIGHT_OFFSET), 0);
  }
}

void
Firefly::reactivate()
{
  if (!GameSession::current()) {
    return;
  }
  if (!GameSession::current()->get_reset_point_sectorname().empty() &&
     GameSession::current()->get_reset_point_pos() == initial_position) {
    // TODO: && GameSession::current()->get_reset_point_sectorname() ==  <sector this firefly is in>
    // GameSession::current()->get_current_sector()->get_name() is not yet initialized.
    // Worst case a resetpoint in a different sector at the same position as the real
    // resetpoint the player is spawning is set to ringing, too. Until we can check the sector, too, dont set
    // activated = true; here.
    m_sprite->set_action("ringing");
  }
}

HitResponse
Firefly::collision(GameObject& other, const CollisionHit& )
{
  // If the bell is already activated, don't ring it again!
  if (activated || m_sprite->get_action() == "ringing")
    return ABORT_MOVE;

  auto player = dynamic_cast<Player*> (&other);
  if (player) {
    activated = true;
    // spawn some particles
    // TODO: provide convenience function in MovingSprite or MovingObject?
    for (int i = 0; i < 5; i++) {
      Vector ppos = m_col.m_bbox.get_middle();
      float angle = graphicsRandom.randf(-math::PI_2, math::PI_2);
      float velocity = graphicsRandom.randf(450.0f, 900.0f);
      float vx = sinf(angle)*velocity;
      float vy = -cosf(angle)*velocity;
      Vector pspeed = Vector(vx, vy);
      Vector paccel = Vector(0.0f, 1000.0f);
      Sector::get().add<SpriteParticle>("images/particles/reset.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_OBJECTS-1);
    }

    if ( m_sprite_name.find("vbell", 0) != std::string::npos ) {
      SoundManager::current()->play("sounds/savebell_low.wav");
    }
    else if ( m_sprite_name.find("torch", 0) != std::string::npos) {
      SoundManager::current()->play("sounds/fire.ogg");
    }
    else {
      SoundManager::current()->play("sounds/savebell2.wav");
    }

    m_sprite->set_action("ringing");
    GameSession::current()->set_reset_point(Sector::get().get_name(),
                                            initial_position);
  }

  return ABORT_MOVE;
}

ObjectSettings
Firefly::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.add_test_from_here();
  return result;
}

void
Firefly::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
