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

#include <variant>

#include "collision/collision_system.hpp"
#include "object/player.hpp"
#include "math/easing.hpp"
#include "supertux/sector.hpp"

namespace {

const float IGEL_SPEED = 80;
const int   IGEL_MAX_DROP_HEIGHT = 16;

const float ROLL_RANGE = 32*10;
const int   ROLL_MAX_DROP_HEIGHT = -1;
const float ROLL_SPEED = 350;
const float ROLL_DURATION = 2.f;
const float ROLL_EASE_TIMER = 0.5f;

} // namespace

Igel::Igel(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/igel/igel.sprite", "left", "right"),
  m_state(STATE_NORMAL),
  m_roll_timer(),
  m_ease_timer()
{
  walk_speed = IGEL_SPEED;
  max_drop_height = IGEL_MAX_DROP_HEIGHT;
}

void
Igel::active_update(float dt_sec)
{
  WalkingBadguy::active_update(dt_sec);
  switch (m_state)
  {
    case STATE_ROLLING:
      if (get_action() == "roll-start-" + dir_to_string(m_dir) &&
          m_sprite->animation_done())
      {
        set_action("roll", m_dir);
      }

      if (m_ease_timer.started())
      {
        float progress = m_ease_timer.get_timegone() / m_ease_timer.get_period();
        float vel = (SineEaseOut(progress) * (ROLL_SPEED - IGEL_SPEED)) + IGEL_SPEED;
        set_walk_speed(vel);
        m_physic.set_velocity_x(vel * (m_dir == Direction::LEFT ? -1 : 1));
      }

      if (m_roll_timer.check())
      {
        if (should_roll())
        {
          // We're still tryna chase them?
          // Then continue chasing them.
          m_roll_timer.start(ROLL_DURATION);
          break;
        }

        stop_rolling();
        break;
      }

      break;

    case STATE_NORMAL:
      if (get_action() == "roll-end-" + dir_to_string(m_dir) &&
          m_sprite->animation_done())
      {
        set_action(m_dir);
      }

      if (m_ease_timer.started())
      {
        float progress = m_ease_timer.get_timegone() / m_ease_timer.get_period();
        float vel = (SineEaseIn(progress) * (IGEL_SPEED - ROLL_SPEED)) + ROLL_SPEED;
        set_walk_speed(vel);
        m_physic.set_velocity_x(vel * (m_dir == Direction::LEFT ? -1 : 1));
      }

      if (should_roll()) roll();

      break;
  }
}

void Igel::collision_solid(const CollisionHit &hit)
{
  if (m_state == STATE_ROLLING)
  {
    if ((hit.left && (m_dir == Direction::LEFT)) || (hit.right && (m_dir == Direction::RIGHT))) {
      stop_rolling();
    }
  }

  WalkingBadguy::collision_solid(hit);
}

HitResponse Igel::collision_badguy(BadGuy &badguy, const CollisionHit &hit)
{
  if (m_state == STATE_ROLLING)
  {
    badguy.kill_fall();
    return FORCE_MOVE;
  }

  return WalkingBadguy::collision_badguy(badguy, hit);
}

GameObjectTypes Igel::get_types() const
{
  return {
    { "normal", _("Normal") },
    { "corrupted", _("Corrupted") }
  };
}

std::string Igel::get_default_sprite_name() const
{
  switch (m_type)
  {
    case NORMAL: return "images/creatures/igel/igel.sprite";
    case CORRUPTED: return "images/creatures/igel/corrupted/corrupted_igel.sprite";
  }
  return "images/creatures/igel/igel.sprite";
}

bool Igel::should_roll()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  Player* plr = get_nearest_player();
  if (!plr) return false;

  Rectf pb = plr->get_bbox();

  bool in_reach_left = (pb.get_right() >= get_bbox().get_right()-((m_dir == Direction::LEFT) ? ROLL_RANGE : 0));
  bool in_reach_right = (pb.get_left() <= get_bbox().get_left()+((m_dir == Direction::RIGHT) ? ROLL_RANGE : 0));
  bool in_reach_top = (pb.get_bottom() >= get_bbox().get_top());
  bool in_reach_bottom = (pb.get_top() <= get_bbox().get_bottom());

  Rectf box = get_bbox().grown(1.f);
  Vector eye = {m_dir == Direction::LEFT ? box.get_left() : box.get_right(), box.get_middle().y};
  bool can_see_player = Sector::get().free_line_of_sight(eye, pb.get_middle(), true);

  return in_reach_left && in_reach_right && in_reach_top && in_reach_bottom && can_see_player;
}

void Igel::roll()
{
  m_state = STATE_ROLLING;
  set_action("roll-start", m_dir);
  //set_walk_speed(ROLL_SPEED);
  //m_physic.set_velocity_x(ROLL_SPEED * (m_dir == Direction::LEFT ? -1 : 1));
  max_drop_height = ROLL_MAX_DROP_HEIGHT;
  m_roll_timer.start(ROLL_DURATION);
  m_ease_timer.start(ROLL_EASE_TIMER);
}

void Igel::stop_rolling()
{
  m_state = STATE_NORMAL;
  set_action("roll-end", m_dir);
  //set_walk_speed(IGEL_SPEED);
  //m_physic.set_velocity_x(IGEL_SPEED * (m_dir == Direction::LEFT ? -1 : 1));
  max_drop_height = IGEL_MAX_DROP_HEIGHT;
  m_roll_timer.stop();
  m_ease_timer.start(ROLL_EASE_TIMER);
}

/* EOF */
