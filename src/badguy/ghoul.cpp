//  SuperTux
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

#include "badguy/ghoul.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const float DEFAULT_SPEED = 40.0f;
static const float DEFAULT_TRACK_RANGE = 2500.0f;
static const float RESPAWN_TIME = 5.f;

Ghoul::Ghoul(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/ghoul/ghoul.sprite"),
  m_speed(),
  m_track_range(),
  m_speed_modifier(),
  m_fake_dead(),
  m_chase_dir(),
  m_respawn_timer(),
  m_sprite_state(SpriteState::NORMAL)
{
  m_countMe = false;

  reader.get("speed", m_speed, DEFAULT_SPEED);
  reader.get("track-range", m_track_range, DEFAULT_TRACK_RANGE);

  m_sprite->set_action(m_dir);
  m_physic.set_gravity_modifier(0.2f);

  SoundManager::current()->preload("sounds/fall.wav");
}

void
Ghoul::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  if (m_frozen)
    return;

  if (m_fake_dead && m_respawn_timer.check()) {
    initialize();
    m_physic.enable_gravity(true);
    set_colgroup_active(COLGROUP_MOVING);
    m_fake_dead = false;
    m_respawn_timer.stop();
  }

  if (m_fake_dead)
    return;

  auto player = get_nearest_player();
  if (!player)
  {
    m_physic.reset();
    return;
  }
  Vector p1 = m_col.m_bbox.get_middle();
  Vector p2 = player->get_bbox().get_middle();
  Vector dist = (p2 - p1);

  switch (m_sprite_state)
  {
  case NORMAL:
    m_speed_modifier = std::max(0.f, m_speed_modifier - (dt_sec * 2.f));
    m_sprite->set_action(player->get_bbox().get_middle().x < get_bbox().get_middle().x ? "normal-left" : "normal-right", 1);
    if (m_sprite->animation_done()) {
      m_chase_dir = glm::normalize(dist);
      m_sprite_state = SpriteState::FAST;
    }
    break;
  case FAST:
    m_sprite->set_action(player->get_bbox().get_middle().x < get_bbox().get_middle().x ? "fast-left" : "fast-right", 1);
    m_speed_modifier = 3.5f;
    if (m_sprite->animation_done()) {
      m_sprite_state = SpriteState::NORMAL;
    }
    break;
  }

  if ((glm::length(dist) >= 1) && (glm::length(dist) < m_track_range))
  {
    m_physic.set_velocity(m_chase_dir * m_speed * m_speed_modifier);
  }
  else
  {
    m_physic.set_velocity(0.f, 0.f);
    m_physic.set_acceleration(0.f, 0.f);
  }
}

HitResponse
Ghoul::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  BadGuy::collision_badguy(badguy, hit);
  return CONTINUE;
}

void
Ghoul::collision_solid(const CollisionHit& hit)
{
// allows it to continue moving if it hits a wall.
}

void
Ghoul::collision_tile(uint32_t tile_attributes)
{
  // don't give it any unique tile interactions, such as hurting on spikes or swimming in water.
}


ObjectSettings
Ghoul::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();

  result.add_float(_("Track Range"), &m_track_range, "track-range", DEFAULT_TRACK_RANGE);
  result.add_float(_("Speed"), &m_speed, "speed", DEFAULT_SPEED);

  result.reorder({ "track-range", "speed", "direction", "x", "y" });

  return result;
}

bool
Ghoul::collision_squished(GameObject& object)
{
  auto player = Sector::get().get_nearest_player(m_col.m_bbox);
  if (player)
    player->bounce(*this);
  kill_fall();
  return true;
}

void
Ghoul::kill_fall()
{
  // "killing" the Ghoul doesn't actually kill it, because it is going to respawn, but it pretends to die.
  set_action("squished");
  SoundManager::current()->play("sounds/fall.wav", get_pos());
  m_physic.enable_gravity(false);
  m_physic.set_velocity(0.f, 0.f);
  m_physic.set_acceleration(0.f, 0.f);
  m_fake_dead = true;
  set_colgroup_active(COLGROUP_DISABLED);
  m_respawn_timer.start(RESPAWN_TIME);
}

/* EOF */
