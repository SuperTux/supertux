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
#include "audio/sound_source.hpp"
#include "math/util.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

Iceflame::Iceflame(const ReaderMapping& reader) :
  Flame(reader, "images/creatures/flame/iceflame.sprite")
{
  m_lightsprite->set_color(Color(0.00f, 0.13f, 0.18f));
}

void
Iceflame::active_update(float dt_sec)
{
  Flame::active_update(dt_sec);
  m_sprite->set_angle(math::degrees(angle) * 3.0f);
}

void
Iceflame::ignite()
{
  SoundManager::current()->play("sounds/sizzle.ogg", get_pos());
  m_sprite->set_action("fade", 1);
  Sector::get().add<SpriteParticle>("images/particles/smoke.sprite",
                                         "default",
                                         m_col.m_bbox.get_middle(), ANCHOR_MIDDLE,
                                         Vector(0, -150), Vector(0,0),
                                         LAYER_BACKGROUNDTILES+2);
  set_group(COLGROUP_DISABLED);

  // start dead-script
  run_dead_script();
}

bool
Iceflame::is_flammable() const
{
  return true;
}

bool
Iceflame::is_freezable() const
{
  return false;
}

/* EOF */
