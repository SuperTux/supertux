//  Zeekling - flyer that swoops down when she spots the player
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "badguy/zeekling.hpp"

#include <math.h>

#include "math/easing.hpp"
#include "math/random.hpp"
#include "math/vector.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

#include <variant>

/*
const float CATCH_DURATION = 0.49f;
const float CATCH_BIG_DISTANCE = 32.f*3.2f; // distance from the ground
const float CATCH_SMALL_DISTANCE = 32.f*2.6f; // same here

const float DIVE_DETECT_STAND = 0.9f;
//const float DIVE_DETECT_DIVIDER = 128.f; // some weird magic number i thought of
*/

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  m_speed(260.f),
  m_catch_pos(0.f),
  m_timer(),
  m_state(FLYING)
{
  m_physic.enable_gravity(false);
}

void Zeekling::draw(DrawingContext &context)
{
  BadGuy::draw(context);
}

void
Zeekling::initialize()
{
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -m_speed : m_speed);
  set_action(m_dir);
}

bool
Zeekling::collision_squished(GameObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
Zeekling::on_bump_horizontal()
{
  m_dir = (m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT);
  set_action(m_dir);
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -m_speed : m_speed);

  switch (m_state)
  {
    case DIVING:
      // Diving attempt failed. So sad. Return to base.
      m_state = RECOVERING;
      m_catch_pos = get_pos().y;
      m_physic.set_velocity_y(0);
      break;

    default:
      break;
  }
}

void
Zeekling::on_bump_vertical()
{
  switch (m_state) {
    case DIVING:
      // Diving attempt failed. So sad. Return to base.
      m_state = RECOVERING;
      m_catch_pos = get_pos().y;
      set_action(m_dir);
      break;

    case RECOVERING:
      // I guess this is my new home now.
      m_state = FLYING;
      m_start_position.y = get_pos().y;
      m_physic.set_velocity_y(0);
      set_action(m_dir);
      break;

    default:
      break;
  }
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    BadGuy::collision_solid(hit);
    return;
  }

  if (BadGuy::get_state() == STATE_SQUISHED ||
      BadGuy::get_state() == STATE_BURNING)
  {
    return;
  }

  if (hit.top || hit.bottom) {
    on_bump_vertical();
  }
  else if (hit.left || hit.right)
  {
    on_bump_horizontal();
  }
}

bool
Zeekling::should_we_dive()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (m_frozen)
    return false;

  Player* player = get_nearest_player();
  if (!player)
    return false;

  // Left/rightmost point of the hitbox.
  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.x = m_dir == Direction::LEFT ? bbox.get_left() : bbox.get_right();

  const Vector& plrmid = player->get_bbox().get_middle();

  // Do not dive if we are not above the player.
  float height = player->get_bbox().get_top() - get_bbox().get_bottom();
  if (height <= 0)
    return false;

  // Do not dive if we are too far above the player.
  if (height > 512)
    return false;

  float dist = std::abs(eye.x - plrmid.x);
  if (!math::in_bounds(dist, 10.f, 32.f * 15))
    return false;

  RaycastResult result = Sector::get().get_first_line_intersection(eye, plrmid, false, nullptr);

  auto* resultobj = std::get_if<CollisionObject*>(&result.hit);
  
  if (result.is_valid && resultobj &&
      *resultobj == player->get_collision_object())
  {
    m_target_y = plrmid.y;
    return true;
  }
  else
  {
    return false;
  }
}

void
Zeekling::active_update(float dt_sec) {
  switch (m_state) {
    case FLYING:
      std::cout << m_start_position.y << " "
                << get_pos().y << std::endl;

      if (!should_we_dive())
        break;

      m_state = DIVING;
      set_action("dive", m_dir);

      //[[fallthrough]];
      break;

    case DIVING:
      if (math::in_bounds(get_bbox().get_bottom(), m_target_y - 5.f, m_target_y + 5.f))
      {
        m_state = CATCHING;
        m_timer.start(1.5f);
        m_physic.set_velocity_y(0.f);
        set_action(m_dir);
        break;
      }
      else
      {
        float startdist = m_target_y - m_start_position.y;
        float dist = m_target_y - get_bbox().get_top();
        float progress = (1.f - (dist / startdist)) * 550.f;
        float value = 550.f - progress;
        /*
        std::cout << m_target_y << " "
                  << dist << " / "
                  << startdist << " "
                  << progress << " "
                  << value << std::endl;
        */

        m_physic.set_velocity_y(value);

        break;
      }

    case CATCHING:
      if (m_timer.check())
      {
        m_state = RECOVERING;
        m_catch_pos = get_pos().y;
      }

      break;

    case RECOVERING:
      if (math::in_bounds(get_bbox().get_bottom(), m_start_position.y - 5.f, m_start_position.y + 5.f))
      {
        m_state = FLYING;
        m_start_position.y = get_pos().y;
        m_physic.set_velocity_y(0);
        set_action(m_dir);
      }
      else
      {
        float startdist = m_catch_pos - m_start_position.y;
        float dist = get_bbox().get_top() - m_start_position.y - 25.f;
        float progress = (dist / startdist) * 550.f;
        float value = progress < 550.f / 2 ? progress : 550.f - progress;
        std::cout << m_catch_pos << " "
                  << m_start_position.y << " "
                  << dist << " / "
                  << startdist << " "
                  << progress << " "
                  << value << std::endl;

        m_physic.set_velocity_y(-value);
      }

      break;

    default:
      break;
  }

  BadGuy::active_update(dt_sec);
}

void
Zeekling::freeze()
{
  BadGuy::freeze();
  m_physic.enable_gravity(true);
}

void
Zeekling::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  m_physic.enable_gravity(false);
  m_state = FLYING;
  initialize();
}

bool
Zeekling::is_freezable() const
{
  return true;
}

/* EOF */
