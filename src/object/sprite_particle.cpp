//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "object/sprite_particle.hpp"

#include "object/camera.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "video/video_system.hpp"
#include "video/viewport.hpp"

SpriteParticle::SpriteParticle(const std::string& sprite_name, const std::string& action,
                               const Vector& position_, AnchorPoint anchor, const Vector& velocity_, const Vector& acceleration_,
                               int drawing_layer_, bool notimeout, Color color) :
  SpriteParticle(SpriteManager::current()->create(sprite_name), action,
                 position_, anchor, velocity_, acceleration_,
                 drawing_layer_, notimeout, color)
{
}

SpriteParticle::SpriteParticle(const SpriteData::LinkedSprite& linked_sprite,
                               const Vector& position_, AnchorPoint anchor, const Vector& velocity_, const Vector& acceleration_,
                               int drawing_layer_, bool notimeout, Color color) :
  SpriteParticle(SpriteManager::current()->create(linked_sprite.file), linked_sprite.config.action.empty() ? "default" : linked_sprite.config.action,
                 position_, anchor, velocity_, acceleration_,
                 drawing_layer_, notimeout, color)
{
}

SpriteParticle::SpriteParticle(SpritePtr sprite_, const std::string& action,
                               const Vector& position_, AnchorPoint anchor, const Vector& velocity_, const Vector& acceleration_,
                               int drawing_layer_, bool notimeout, Color color) :
  sprite(std::move(sprite_)),
  position(position_),
  velocity(velocity_),
  acceleration(acceleration_),
  drawing_layer(drawing_layer_),
  lightsprites(),
  no_time_out(false)
{
  sprite->set_action(action, 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action
  sprite->set_color(color);

  position -= get_anchor_pos(sprite->get_current_hitbox(), anchor);
  no_time_out = notimeout;

  for (const auto& sprite_data : sprite->get_custom_linked_sprites())
  {
    if (!sprite_data.light)
      continue;

    SpritePtr sprite = SpriteManager::current()->create(sprite_data.file);
    sprite->apply_config(sprite_data.config);
    sprite->set_blend(Blend::ADD);
    if (sprite_data.config.color == Color(1.0f, 1.0f, 1.0f, 1.0f))
      sprite->set_color(color);

    lightsprites.push_back(std::move(sprite));
  }
}



SpriteParticle::~SpriteParticle()
{
  remove_me();
}

void
SpriteParticle::update(float dt_sec)
{
  // die when animation is complete
  if (sprite->animation_done() && !no_time_out) {
    remove_me();
    return;
  }

  // calculate new position and velocity
  position.x += velocity.x * dt_sec;
  position.y += velocity.y * dt_sec;
  velocity.x += acceleration.x * dt_sec;
  velocity.y += acceleration.y * dt_sec;

  // die when too far offscreen
  Camera& camera = Sector::get().get_camera();
  if (!camera.get_rect().contains(position)) {
    remove_me();
  }
}

void
SpriteParticle::draw(DrawingContext& context)
{
  Vector draw_pos = position + velocity * context.get_time_offset();
  sprite->draw(context.color(), draw_pos, drawing_layer);

  if (!lightsprites.empty())
  {
    sprite->draw(context.light(), draw_pos, drawing_layer);

    draw_pos += Vector(12.f, 12.f);
    for (auto& sprite : lightsprites)
      sprite->draw(context.light(), draw_pos, 0);
  }
}

/* EOF */
