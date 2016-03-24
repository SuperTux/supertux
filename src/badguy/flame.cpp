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
#include "editor/editor.hpp"
#include "math/random_generator.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const std::string FLAME_SOUND = "sounds/flame.wav";

Flame::Flame(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/flame/flame.sprite", LAYER_FLOATINGOBJECTS,
         "images/objects/lightmap_light/lightmap_light-small.sprite"),
  angle(0),
  radius(),
  speed(),
  sound_source()
{
  if ( !reader.get("radius", radius)) radius = 100;
  if ( !reader.get("speed", speed)) speed = 2;
  if (!EditorActive()) {
    bbox.set_pos(Vector(start_position.x + cos(angle) * radius,
                        start_position.y + sin(angle) * radius));
  }
  countMe = false;
  SoundManager::current()->preload(FLAME_SOUND);

  set_colgroup_active(COLGROUP_TOUCHABLE);

  lightsprite->set_color(Color(0.21f, 0.13f, 0.08f));
  glowing = true;
}

void
Flame::save(Writer& writer) {
  BadGuy::save(writer);
  writer.write("radius", radius);
  writer.write("speed", speed);
}

ObjectSettings
Flame::get_settings() {
  ObjectSettings result = BadGuy::get_settings();
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Radius"), &radius));
  result.options.push_back( ObjectOption(MN_NUMFIELD, _("Speed"), &speed));
  return result;
}

void
Flame::active_update(float elapsed_time)
{
  angle = fmodf(angle + elapsed_time * speed, (float) (2*M_PI));
  Vector newpos(start_position.x + cos(angle) * radius,
                start_position.y + sin(angle) * radius);
  if (!EditorActive()) {
    movement = newpos - get_pos();
    sound_source->set_position(get_pos());
  }

  if (sprite->get_action() == "fade" && sprite->animation_done()) remove_me();
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
  Sector::current()->add_object(std::make_shared<SpriteParticle>("images/objects/particles/smoke.sprite",
                                                                 "default",
                                                                 bbox.get_middle(), ANCHOR_MIDDLE,
                                                                 Vector(0, -150), Vector(0,0), LAYER_BACKGROUNDTILES+2));
  set_group(COLGROUP_DISABLED);

  // start dead-script
  run_dead_script();
}

bool
Flame::is_freezable() const
{
  return true;
}

bool
Flame::is_flammable() const
{
  return false;
}

void Flame::stop_looping_sounds()
{
  if (sound_source) {
    sound_source->stop();
  }
}

void Flame::play_looping_sounds()
{
  if (sound_source) {
    sound_source->play();
  }
}

/* EOF */
