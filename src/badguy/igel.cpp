//  SuperTux - Badguy "Igel"
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "badguy/igel.hpp"

#include "math/random.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "object/sprite_particle.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

static const float FOUND_DISTANCE = 256.0f;
static const float FOUND_TIME = 0.6f;
static const float MAX_ROLL_SPEED = 250.f;
static const float LOST_DISTANCE = 400.0f;

Igel::Igel(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/igel/igel.sprite", "walking-left", "walking-right"),
  m_state(IgelState::WALKING),
  m_found_timer()
{
  walk_speed = 80.f;
  max_drop_height = 16;
}

void
Igel::active_update(float dt_sec)
{
  Player* player = get_nearest_player();
  if (!player)
    return;
  Rectf pb = player->get_bbox();
  float player_dir = pb.get_middle().x > get_bbox().get_middle().x ? 1.f : -1.f;

  bool inReach_left = (pb.get_right() >= m_col.m_bbox.get_right() - ((m_dir == Direction::LEFT) ? FOUND_DISTANCE : 0));
  bool inReach_right = (pb.get_left() <= m_col.m_bbox.get_left() + ((m_dir == Direction::RIGHT) ? FOUND_DISTANCE : 0));
  bool inReach_top = (pb.get_bottom() >= m_col.m_bbox.get_top());
  bool inReach_bottom = (pb.get_top() <= m_col.m_bbox.get_bottom());

  switch (m_state) {
  case WALKING:
    if (player && inReach_left && inReach_right && inReach_top && inReach_bottom && !m_frozen)
    {
      // found
      m_found_timer.start(FOUND_TIME);
      m_physic.set_velocity(0.f, -250.f);
      m_sprite->set_action("found", m_dir, 1);
      m_state = IgelState::FOUND;
    }
    WalkingBadguy::active_update(dt_sec);
    break;
  case FOUND:
    m_physic.set_velocity_x(0.f);
    if (m_found_timer.check())
    {
      max_drop_height = -1;
      m_found_timer.stop();
      m_sprite->set_action("rolling", m_dir, -1);
      m_state = IgelState::ROLLING;
    }
    BadGuy::active_update(dt_sec);
    break;
  case ROLLING:
    m_sprite->set_angle(m_sprite->get_angle() + m_physic.get_movement(dt_sec).x * 3.141592653898f);
    WalkingBadguy::active_update(dt_sec, MAX_ROLL_SPEED * player_dir, 1.5f);

    if (glm::length(pb.get_middle() - get_bbox().get_middle()) > LOST_DISTANCE || m_frozen)
    {
      max_drop_height = 16;
      m_sprite->set_action(m_frozen ? "iced" : "walking", m_dir, -1);
      m_sprite->set_angle(0.f);
      m_state = IgelState::WALKING;
    }
    break;
  }
}

HitResponse
Igel::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (m_frozen)
    return FORCE_MOVE;
  if (m_state == IgelState::ROLLING)
  {
    badguy.kill_fall();
    return FORCE_MOVE;
  }
  else
  {
    WalkingBadguy::collision_badguy(badguy, hit);
  }
  return ABORT_MOVE;
}

void
Igel::collision_solid(const CollisionHit& hit)
{
  WalkingBadguy::collision_solid(hit);
  if (m_frozen)
  {
    return;
  }

  switch (m_state)
  {
  case WALKING:
  case FOUND:
    break;
  case ROLLING:
    if (hit.left || hit.right)
    {
      for (int i = 0; i < 5; i++) {
        float angle = graphicsRandom.randf(hit.left ? -90.f : 90.f, hit.left ? 90.f : 270.f);
        Vector speed = graphicsRandom.randf(50.f, 200.f) * Vector(std::cos(math::radians(angle)), std::sin(math::radians(angle)));
        Vector accel = Vector(0.f, 800.f);
        Sector::get().add<SpriteParticle>("images/particles/generic_piece_small.sprite", "default",
          Vector(hit.left ? get_bbox().get_left() : get_bbox().get_right(), get_bbox().get_middle().y), ANCHOR_MIDDLE,
          speed, accel, LAYER_OBJECTS + 6);
      }
    }
    break;
  }
}

/* EOF */
