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

#include "badguy/iceflame.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

Iceflame::Iceflame(const Reader& reader) :
  BadGuy(reader, "images/creatures/flame/iceflame.sprite", LAYER_FLOATINGOBJECTS), 
  angle(0), 
  radius(100), 
  speed(2),
  fading(false),
  light(0.0f,0.0f,0.0f),
  lightsprite(sprite_manager->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  reader.get("radius", radius);
  reader.get("speed", speed);
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  countMe = false;
  //TODO: get unique death sound
  sound_manager->preload("sounds/fizz.wav");
  
  set_colgroup_active(COLGROUP_TOUCHABLE);
  
  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.00f, 0.13f, 0.18f));
  
}

void
Iceflame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();
  
  if(fading)
    if (sprite->animation_done()) remove_me();
}

void
Iceflame::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  //Draw the light if dark
  context.get_light( get_bbox().get_middle(), &light );
  if (light.blue + light.green < 2.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    lightsprite->draw(context, get_bbox().get_middle(), 0);
    context.pop_target();
  }
}


void
Iceflame::kill_fall()
{
}

void
Iceflame::ignite()
{
  sound_manager->play("sounds/fizz.wav", get_pos());
  sprite->set_action("fade", 1);
  Vector ppos = bbox.get_middle();
  Vector pspeed = Vector(0, -150);
  Vector paccel = Vector(0,0);
  Sector::current()->add_object(new SpriteParticle("images/objects/particles/smoke.sprite", "default", ppos, ANCHOR_MIDDLE, pspeed, paccel, LAYER_BACKGROUNDTILES+2));
  fading = true;
  
  // start dead-script
  run_dead_script();
}

bool
Iceflame::is_flammable() const
{
  return true;
}

/* EOF */
