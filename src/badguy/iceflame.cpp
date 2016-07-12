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
#include "util/reader_mapping.hpp"

Iceflame::Iceflame(const ReaderMapping& reader) :
  Flame(reader)
{
  lightsprite->set_color(Color(0.00f, 0.13f, 0.18f));
  sprite = SpriteManager::current()->create("images/creatures/flame/iceflame.sprite");
}

void
Iceflame::active_update(float elapsed_time)
{
  Flame::active_update(elapsed_time);
  sprite->set_angle(angle * 360.0f / (2*M_PI) * 3);
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

bool
Iceflame::is_freezable() const
{
  return false;
}

/* EOF */
