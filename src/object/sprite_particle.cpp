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

#include "object/camera.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/globals.hpp"
#include "supertux/sector.hpp"

#include <stdexcept>

SpriteParticle::SpriteParticle(std::string sprite_name, std::string action,
                               Vector position_, AnchorPoint anchor, Vector velocity_, Vector acceleration_,
                               int drawing_layer_) :
  sprite(),
  position(position_),
  velocity(velocity_),
  acceleration(acceleration_),
  drawing_layer(drawing_layer_),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-tiny.sprite")),
  glow(false)
{
  sprite = SpriteManager::current()->create(sprite_name);
  if (!sprite.get()) throw std::runtime_error("Could not load sprite "+sprite_name);
  sprite->set_action(action, 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action

  this->position -= get_anchor_pos(sprite->get_current_hitbox(), anchor);

  if(sprite_name=="images/objects/particles/sparkle.sprite")
    glow = true;
    if(action=="dark") {
      lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
      lightsprite->set_color(Color(0.1f, 0.1f, 0.1f));
    }
}

SpriteParticle::~SpriteParticle()
{
  remove_me();
}

void
SpriteParticle::hit(Player& )
{
}

void
SpriteParticle::update(float elapsed_time)
{
  // die when animation is complete
  if (sprite->animation_done()) {
    remove_me();
    return;
  }

  // calculate new position and velocity
  position.x += velocity.x * elapsed_time;
  position.y += velocity.y * elapsed_time;
  velocity.x += acceleration.x * elapsed_time;
  velocity.y += acceleration.y * elapsed_time;

  // die when too far offscreen
  Vector camera = Sector::current()->camera->get_translation();
  if ((position.x < camera.x - 128) || (position.x > SCREEN_WIDTH + camera.x + 128) ||
      (position.y < camera.y - 128) || (position.y > SCREEN_HEIGHT + camera.y + 128)) {
    remove_me();
    return;
  }
}

void
SpriteParticle::draw(DrawingContext& context)
{
  sprite->draw(context, position, drawing_layer);

  //Sparkles glow in the dark
  if(glow){
    context.get_light(position, &light );
    if (light.red + light.green + light.blue < 3.0){
      context.push_target();
      context.set_target(DrawingContext::LIGHTMAP);
      sprite->draw(context, position, drawing_layer);
      lightsprite->draw(context, position + Vector(12,12), 0);
      context.pop_target();
    }
  }

}

/* EOF */
