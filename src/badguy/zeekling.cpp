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
  m_speed(160.f),
  m_easing_progress(0.0),
  m_timer(),
  state(FLYING)
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

  if (state == DIVING)
  {
    state = FLYING;
    m_easing_progress = 0.0;
    m_physic.set_velocity_y(0);
  }
  else
  {
    state = RECOVERING;
    m_easing_progress = 0.0;
    m_physic.set_velocity_y(-m_speed);
  }
}

void
Zeekling::on_bump_vertical()
{
  if (BadGuy::get_state() == STATE_BURNING)
  {
    m_physic.set_velocity(0, 0);
    return;
  }

  switch (state) {

    case DIVING:
      state = RECOVERING;
      m_easing_progress = 0.0;
      set_action(m_dir);
      break;

    case RECOVERING:
      state = FLYING;
      break;

    default:
      break;
  }

  m_physic.set_velocity_y(state == RECOVERING ? -m_speed : 0);
}

void
Zeekling::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    BadGuy::collision_solid(hit);
    return;
  }

  if (m_sprite->get_action() == "squished-left" ||
      m_sprite->get_action() == "squished-right")
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

  if (m_frozen) return false;

  Player* plr = get_nearest_player();
  if (!plr) return false;

  // Left/rightmost point of the hitbox.
  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.x = m_dir == Direction::LEFT ? bbox.get_left() : bbox.get_right();

  const Vector& plrmid = plr->get_bbox().get_middle();

  // Do not dive if we are not above the player.
  float height = plr->get_bbox().get_top() - get_bbox().get_bottom();
  if (height <= 0) return false;

  // Do not dive if we are too far above the player.
  if (height > 512) return false;

  RaycastResult result = Sector::get().get_first_line_intersection(eye, plrmid, false, nullptr);

  auto* resultobj = std::get_if<CollisionObject*>(&result.hit);
  
  if (result.is_valid && resultobj &&
      *resultobj == plr->get_collision_object())
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
  switch (state) {
    case FLYING:
      if (!should_we_dive())
        break;

      state = DIVING;
      set_action("dive", m_dir);

      [[fallthrough]];

    case DIVING:
      if (get_bbox().get_bottom() == m_target_y)
      {
        state = FLYING;
        //m_timer.start(1.f);
        set_action(m_dir);
        break;
      }
      else
      {
        float startdist = std::abs(m_target_y - m_start_position.y);
        float dist = std::abs(m_target_y - get_bbox().get_top());
        double progress = static_cast<double>(dist / startdist);
        float value = std::max(1.f, 0.15f * dist * static_cast<float>(QuadraticEaseOut(progress)));
        std::cout << dist << " / "
                  << startdist << " "
                  << progress << " "
                  << value << std::endl;

        set_pos({get_pos().x, get_pos().y + value});

        break;
      }

    case RECOVERING: {
      if (m_timer.check())
      {
        state = FLYING;
        m_physic.set_velocity_y(0);
        set_action(m_dir);
      }
      else
      {
        double progress = static_cast<double>(m_timer.get_progress() / 2);
        float dist = -std::abs(m_start_position.y - get_bbox().get_top());
        float value = dist * static_cast<float>(QuadraticEaseInOut(progress));

        m_physic.set_velocity_y(value);
      }

      break;
    }

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
  state = FLYING;
  initialize();
}

bool
Zeekling::is_freezable() const
{
  return true;
}

/* EOF */
