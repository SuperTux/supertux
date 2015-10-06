//  SuperTux badguy - Ghostflame a flame-like enemy that cannot be killed
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#include "badguy/ghostflame.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader.hpp"

Ghostflame::Ghostflame(const Reader& reader) :
  BadGuy(reader, "images/creatures/flame/ghostflame.sprite", LAYER_FLOATINGOBJECTS),
  angle(0),
  radius(100),
  speed(2),
  light(0.0f,0.0f,0.0f),
  lightsprite(SpriteManager::current()->create("images/objects/lightmap_light/lightmap_light-small.sprite"))
{
  reader.get("radius", radius);
  reader.get("speed", speed);
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  countMe = false;
  //TODO: get unique death sound
  SoundManager::current()->preload("sounds/fizz.wav");

  set_colgroup_active(COLGROUP_TOUCHABLE);

  lightsprite->set_blend(Blend(GL_SRC_ALPHA, GL_ONE));
  lightsprite->set_color(Color(0.21f, 0.00f, 0.21f));

}

void
Ghostflame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();

}

void
Ghostflame::draw(DrawingContext& context)
{
  //Draw the Sprite.
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  //Draw the light if dark
  context.get_light( bbox.get_middle(), &light );
  if (light.blue + light.red < 2.0){
    context.push_target();
    context.set_target(DrawingContext::LIGHTMAP);
    sprite->draw(context, get_pos(), layer);
    lightsprite->draw(context, bbox.get_middle(), 0);
    context.pop_target();
  }
}


void
Ghostflame::kill_fall()
{
}

ObjectSettings
Ghostflame::get_settings() {
  ObjectSettings result(_("Ghost flame"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Death script"), &dead_script));
  return result;
}

bool
Ghostflame::is_flammable() const
{
  return false;
}

/* EOF */
