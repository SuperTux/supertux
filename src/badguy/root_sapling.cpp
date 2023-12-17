//  SuperTux - Root Sapling Badguy
//  Copyright (C) 2023 MatusGuy
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

#include "badguy/root_sapling.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/root.hpp"
#include "math/random.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"

static const float ROOT_SAPLING_RANGE = 32.f*5;
static const float ROOT_SAPLING_SPAWN_TIME = 1.f;

RootSapling::RootSapling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/corrupted/root_sapling.sprite", LAYER_TILES-1),
  m_root_timer(),
  m_dead(false)
{
  m_physic.enable_gravity(false);
  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
}

void
RootSapling::kill_fall()
{
  m_dead = true;
  SoundManager::current()->play("sounds/fall.wav", get_pos());
  run_dead_script();
}

HitResponse
RootSapling::collision_badguy(BadGuy&, const CollisionHit&)
{
  return FORCE_MOVE;
}

bool
RootSapling::collision_squished(GameObject&)
{
  m_dead = true;
  SoundManager::current()->play("sounds/squish.wav", get_pos());
  run_dead_script();
  return true;
}

void
RootSapling::active_update(float dt_sec)
{
  if (m_dead) return;

  BadGuy::active_update(dt_sec);

  Player* player = get_nearest_player();
  if (!player) return;

  float distance = std::abs(get_pos().x - player->get_pos().x);
  if (distance > ROOT_SAPLING_RANGE) return;

  if (!m_root_timer.started() || m_root_timer.check())
  {
    summon_root();
    m_root_timer.start(ROOT_SAPLING_SPAWN_TIME);
  }
}

std::vector<Direction>
RootSapling::get_allowed_directions() const
{
  return {};
}

void
RootSapling::summon_root()
{
  Player* player = get_nearest_player();
  if (!player) return;

  Vector pos = {player->get_pos().x, get_bbox().get_bottom()};
  Root& root = Sector::get().add<Root>(pos, "images/creatures/mole/corrupted/root.sprite");
}

void
RootSapling::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
