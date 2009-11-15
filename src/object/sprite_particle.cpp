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
#include "object/sprite_particle.hpp"
#include "supertux/main.hpp"
#include "supertux/sector.hpp"

SpriteParticle::SpriteParticle(std::string sprite_name, std::string action, Vector position, AnchorPoint anchor, Vector velocity, Vector acceleration, int drawing_layer)
        : position(position), velocity(velocity), acceleration(acceleration), drawing_layer(drawing_layer)
{
  sprite = sprite_manager->create(sprite_name);
  if (!sprite) throw std::runtime_error("Could not load sprite "+sprite_name);
  sprite->set_action(action, 1);
  sprite->set_animation_loops(1); //TODO: this is necessary because set_action will not set "loops" when "action" is the default action

  this->position -= get_anchor_pos(sprite->get_current_hitbox(), anchor);
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
}

/* EOF */
