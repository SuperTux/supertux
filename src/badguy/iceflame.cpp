//  SuperTux badguy - Iceflame a flame-like enemy that can be killed with fireballs
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

#include "badguy/iceflame.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/gettext.hpp"
#include "util/reader_mapping.hpp"

Iceflame::Iceflame(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/flame/iceflame.sprite", LAYER_FLOATINGOBJECTS,
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
  SoundManager::current()->preload("sounds/sizzle.ogg");

  set_colgroup_active(COLGROUP_TOUCHABLE);

  lightsprite->set_color(Color(0.00f, 0.13f, 0.18f));
  glowing = true;

}

void
Iceflame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  movement = newpos - get_pos();
  sprite->set_angle(angle * 360.0f / (2*M_PI) * 3);

  if (sprite->get_action() == "fade" && sprite->animation_done()) remove_me();
}

void
Iceflame::kill_fall()
{
}

void
Iceflame::ignite()
{
  SoundManager::current()->play("sounds/sizzle.ogg", get_pos());
  sprite->set_action("fade", 1);
  Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/smoke.sprite",
                                                                 "default",
                                                                 bbox.get_middle(), ANCHOR_MIDDLE,
                                                                 Vector(0, -150), Vector(0,0),
                                                                 LAYER_BACKGROUNDTILES+2));
  set_group(COLGROUP_DISABLED);

  // start dead-script
  run_dead_script();
}

bool
Iceflame::is_flammable() const
{
  return true;
}

ObjectSettings
Iceflame::get_settings() {
  ObjectSettings result(_("Ice flame"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( dir_option(&dir) );
  result.options.push_back( ObjectOption(MN_SCRIPT, _("Death script"), &dead_script));
  return result;
}

/* EOF */
