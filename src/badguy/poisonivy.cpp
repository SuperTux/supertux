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

#include "badguy/poisonivy.hpp"

#include <math.h>

#include "math/random.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"

PoisonIvy::PoisonIvy(const ReaderMapping& reader)
  : WalkingBadguy(reader, "images/creatures/poison_ivy/poison_ivy.sprite", "left", "right")
 {
  walk_speed = 80;
 }

PoisonIvy::PoisonIvy(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/poison_ivy/poison_ivy.sprite", "left", "right")
 {
  walk_speed = 80;
 }

void
PoisonIvy::initialize()
{
  m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");
  m_physic.set_gravity_modifier(.10f);
  m_physic.enable_gravity(true);
  
}

void
PoisonIvy::active_update(float dt_sec)
{
  if (!on_ground()) {
  m_sprite->set_action(m_dir == Direction::LEFT ? "falling-left" : "falling-right");} else if (on_ground()) {
  m_sprite->set_action(m_dir == Direction::LEFT ? "left" : "right");}
  WalkingBadguy::active_update(dt_sec);
}

bool
PoisonIvy::is_freezable() const
{
  return true;
}

bool
PoisonIvy::collision_squished(GameObject& object)
{
  m_sprite->set_action(m_dir == Direction::LEFT ? "squished-left" : "squished-right");
  // Spawn death particles
  spawn_explosion_sprites(3, "images/objects/particles/poisonivy.sprite");
  kill_squished(object);
  return true;
}

/* EOF */
