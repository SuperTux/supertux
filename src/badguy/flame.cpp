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

#include "audio/sound_manager.hpp"
#include "editor/editor.hpp"
#include "math/util.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const std::string FLAME_SOUND = "sounds/flame.wav";

Flame::Flame(const ReaderMapping& reader, const std::string& sprite) :
  BadGuy(reader, sprite, LAYER_FLOATINGOBJECTS,
         "images/objects/lightmap_light/lightmap_light-small.sprite"),
  angle(0),
  radius(),
  speed(),
  sound_source()
{
  reader.get("radius", radius, 100.0f);
  reader.get("speed", speed, 2.0f);
  if (!Editor::is_active()) {
    m_col.m_bbox.set_pos(Vector(m_start_position.x + cosf(angle) * radius,
                                m_start_position.y + sinf(angle) * radius));
  }
  m_countMe = false;
  SoundManager::current()->preload(FLAME_SOUND);

  set_colgroup_active(COLGROUP_TOUCHABLE);

  m_lightsprite->set_color(Color(0.21f, 0.13f, 0.08f));
  m_glowing = true;
}

ObjectSettings
Flame::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Radius"), &radius, "radius", 100.0f);
  result.add_float(_("Speed"), &speed, "speed", 2.0f);

  result.reorder({"speed", "sprite", "x", "y"});

  return result;
}

void
Flame::active_update(float dt_sec)
{
  angle = fmodf(angle + dt_sec * speed, math::TAU);
  if (!Editor::is_active()) {
    Vector newpos(m_start_position.x + cosf(angle) * radius,
                  m_start_position.y + sinf(angle) * radius);
    m_col.set_movement(newpos - get_pos());
    sound_source->set_position(get_pos());
  }

  if (m_sprite->get_action() == "fade" && m_sprite->animation_done()) remove_me();
}

void
Flame::activate()
{
  if (Editor::is_active())
    return;
  sound_source = SoundManager::current()->create_sound_source(FLAME_SOUND);
  sound_source->set_position(get_pos());
  sound_source->set_looping(true);
  sound_source->set_gain(1.0f);
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
  set_action("fade", 1);
  Sector::get().add<SpriteParticle>("images/particles/smoke.sprite",
                                         "default",
                                         m_col.m_bbox.get_middle(), ANCHOR_MIDDLE,
                                         Vector(0, -150), Vector(0,0), LAYER_BACKGROUNDTILES+2);
  set_group(COLGROUP_DISABLED);

  // Start the dead-script.
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

void
Flame::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
