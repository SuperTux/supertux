//  SuperTux
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

#include "badguy/corrupted_granito_big.hpp"

#include <variant>

#include "audio/sound_manager.hpp"
#include "collision/collision_system.hpp"
#include "object/player.hpp"
#include "object/shard.hpp"
#include "supertux/sector.hpp"

const std::string SHARD_SPRITE = "images/creatures/granito/corrupted/big/root_spike.sprite";

CorruptedGranitoBig::CorruptedGranitoBig(const ReaderMapping& reader):
  BadGuy(reader, "images/creatures/granito/corrupted/big/rock_mine.sprite"),
  m_crack_timer(),
  m_dead(false)
{
  parse_type(reader);

  m_col.set_unisolid(true);

  SoundManager::current()->preload("sounds/brick.wav");
}

void CorruptedGranitoBig::initialize()
{
  BadGuy::initialize();

  set_action("idle", m_dir);
  set_colgroup_active(COLGROUP_MOVING_STATIC);
}

void CorruptedGranitoBig::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  if (m_dead == true) return;

  bool crack = try_cracking();

  if (!m_crack_timer.started()) m_crack_timer.start(2.f);

  if (crack && m_crack_timer.paused()) m_crack_timer.unpause();
  if (!crack && !m_crack_timer.paused()) m_crack_timer.pause();
}

void CorruptedGranitoBig::kill_fall()
{
  set_action("broken", m_dir);
  set_colgroup_active(COLGROUP_MOVING_ONLY_STATIC);
  m_col.set_unisolid(false);

  m_dead = true;

  run_dead_script();

  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(100.f, -500.f), SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(270.f, -350.f), SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-100.f, -500.f),SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-270.f, -350.f),SHARD_SPRITE);
}

bool CorruptedGranitoBig::try_cracking()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  Player* plr = get_nearest_player();
  if (!plr) return false;

  Vector mid = get_bbox().get_middle();
  Vector plrmid = plr->get_bbox().get_middle();

  RaycastResult result = Sector::get().get_first_line_intersection(get_bbox().get_middle(),
                                                                   plr->get_bbox().get_middle(),
                                                                   false,
                                                                   get_collision_object());

  auto* obj = std::get_if<CollisionObject*>(&result.hit);
  if (!obj || *obj != plr->get_collision_object()) return false;

  if (glm::distance(mid, plrmid) > 32.f*4.f) return false;

  crack();
  return true;
}

void CorruptedGranitoBig::crack()
{
  if (m_crack_timer.paused()) return;

  if (m_crack_timer.check())
  {
    kill_fall();
    return;
  }

  float progress = m_crack_timer.get_timegone() / m_crack_timer.get_period();
  std::cout << progress << std::endl;

  if (progress >= 0.5)
  {
    set_action("cracked-2", m_dir);
    return;
  }

  set_action("cracked-1", m_dir);
}

/* EOF */
