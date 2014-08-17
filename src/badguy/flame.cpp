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

#include "badguy/flame.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

static const std::string FLAME_SOUND = "sounds/flame.wav";

Flame::Flame(const Reader& reader) :
  BadGuy(reader, "images/creatures/flame/flame.sprite", LAYER_FLOATINGOBJECTS),
  angle(0),
  radius(100),
  speed(2),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite")),
  sound_source()
{
  reader.get("radius", radius);
  reader.get("speed", speed);
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  countMe = false;
  SoundManager::current()->preload(FLAME_SOUND);

  set_colgroup_active(COLGROUP_TOUCHABLE);

  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.21f, 0.13f, 0.08f));
}

void
Flame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();

  sound_source->set_position(get_pos());

  if (sprite->get_action() == "fade" && sprite->animation_done()) remove_me();
}

void
Flame::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  //Draw the light if dark
  if(true){
    context.get_light( get_bbox().get_middle(), &light );
    if (light.red + light.green < 2.0){
      context.push_target();
      context.set_target(DrawingContext::LIGHTMAP);
      sprite->draw(context, get_pos(), layer);
      lightsprite->draw(context, get_bbox().get_middle(), 0);
      context.pop_target();
    }
  }
}

void
Flame::activate()
{
  sound_source = SoundManager::current()->create_sound_source(FLAME_SOUND);
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(2.0);
  sound_source->set_reference_distance(32);
  sound_source->play();
}

void
Flame::deactivate()
{
  sound_source.reset();
}


void
Flame::kill_fall()
{
}

void
Flame::freeze()
{
  SoundManager::current()->play("sounds/sizzle.ogg", get_pos());
  sprite->set_action("fade", 1);
  Sector::current()->add_object(new SpriteParticle("images/objects/particles/smoke.sprite", "default", bbox.get_middle(), ANCHOR_MIDDLE, Vector(0, -150), Vector(0,0), LAYER_BACKGROUNDTILES+2));
  set_group(COLGROUP_DISABLED);

  // start dead-script
  run_dead_script();
}

bool
Flame::is_freezable() const
{
  return true;
}

/* EOF */
