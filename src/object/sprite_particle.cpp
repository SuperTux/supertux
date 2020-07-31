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
                               int drawing_layer_) :
  SpriteParticle(SpriteManager::current()->create(sprite_name), action,
                 position_, anchor, velocity_, acceleration_,
                 drawing_layer_)
{
  if (sprite_name == "images/particles/sparkle.sprite")
  {
    glow = true;
    if (action=="dark") {
      lightsprite->set_blend(Blend::ADD);
      lightsprite->set_color(Color(0.1f, 0.1f, 0.1f));
    }
  }
}

SpriteParticle::SpriteParticle(SpritePtr sprite_, const std::string& action,
                               const Vector& position_, AnchorPoint anchor, const Vector& velocity_, const Vector& acceleration_,
                               int drawing_layer_) :
  sprite(std::move(sprite_)),
  position(position_),
  velocity(velocity_),
  acceleration(acceleration_),
  drawing_layer(drawing_layer_),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-tiny.sprite")),
  glow(false)
{
  sprite->set_action(action, 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action

  position -= get_anchor_pos(sprite->get_current_hitbox(), anchor);
}

SpriteParticle::~SpriteParticle()
{
  remove_me();
}

void
SpriteParticle::update(float dt_sec)
{
  // die when animation is complete
  if (sprite->animation_done()) {
    remove_me();
    return;
  }

  // calculate new position and velocity
  position.x += velocity.x * dt_sec;
  position.y += velocity.y * dt_sec;
  velocity.x += acceleration.x * dt_sec;
  velocity.y += acceleration.y * dt_sec;

  // die when too far offscreen
  Vector camera = Sector::get().get_camera().get_translation();
  if ((position.x < camera.x - 128.0f) || (position.x > static_cast<float>(SCREEN_WIDTH) + camera.x + 128.0f) ||
      (position.y < camera.y - 128.0f) || (position.y > static_cast<float>(SCREEN_HEIGHT) + camera.y + 128.0f)) {
    remove_me();
    return;
  }
}

void
SpriteParticle::draw(DrawingContext& context)
{
  sprite->draw(context.color(), position, drawing_layer);

  //Sparkles glow in the dark
  if (glow)
  {
    sprite->draw(context.light(), position, drawing_layer);
    lightsprite->draw(context.light(), position + Vector(12,12), 0);
  }

}

/* EOF */
