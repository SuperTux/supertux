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
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

Ghostflame::Ghostflame(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/flame/ghostflame.sprite", LAYER_FLOATINGOBJECTS,
         "images/objects/lightmap_light/lightmap_light-small.sprite"),
  angle(0),
  radius(),
  speed(),
  light(0.0f,0.0f,0.0f)
{
  if ( !reader.get("radius", radius)) radius = 100;
  if ( !reader.get("speed", speed)) speed = 2;
  bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                      start_position.y + sin(angle) * radius));
  countMe = false;
  //TODO: get unique death sound
  SoundManager::current()->preload("sounds/fizz.wav");

  set_colgroup_active(COLGROUP_TOUCHABLE);

  lightsprite->set_color(Color(0.21f, 0.00f, 0.21f));
  glowing = true;

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
Ghostflame::kill_fall()
{
}

bool
Ghostflame::is_flammable() const
{
  return false;
}

/* EOF */
