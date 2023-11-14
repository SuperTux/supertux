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

const float CATCH_DURATION = 0.49f;
const float CATCH_DISTANCE = 32.f*3.2f; // distance from the ground

const float DIVE_DETECT_DIR = 2.1f;
const float DIVE_DIR = 2.5f;

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  speed(160.f),
  m_original_xvel(0.f),
  m_original_yvel(0.f),
  m_easing_progress(0.0),
  m_charge_timer(),
  state(FLYING)
{
  m_physic.enable_gravity(false);
}

void Zeekling::draw(DrawingContext &context)
{
  BadGuy::draw(context);

  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.y = bbox.get_bottom();

  const Vector rangeend = {eye.x, eye.y + CATCH_DISTANCE};

  context.color().draw_line(eye, rangeend, Color::from_rgba8888(0xff, 0,0,0xff), 100);
}

void
Zeekling::initialize()
{
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);
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
  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -speed : speed);

  if (state == DIVING)
  {
    state = FLYING;
    m_physic.set_velocity_y(0);
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
    case CHARGING:
      state = FLYING;
      m_physic.set_velocity_y(0.f);
      m_easing_progress = 0.0;
      set_action(m_dir);
      break;

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

  m_physic.set_velocity_y(state == RECOVERING ? -speed : 0);
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

  // Left/rightmost point of the hitbox.
  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.x = m_dir == Direction::LEFT ? bbox.get_left() : bbox.get_right();

  const Vector& plrmid = get_nearest_player()->get_bbox().get_middle();

  // Do not dive if we are not above the player.
  float height = plrmid.y - eye.y;
  if (height <= 0) return false;

  // Do not dive if we are too far above the player.
  if (height > 512) return false;

  const Vector rangeend = {eye.x + ((plrmid.y - eye.y) * DIVE_DETECT_DIR *
                                     (m_dir == Direction::LEFT ? -1 : 1)),
                            plrmid.y};

  RaycastResult result = Sector::get().get_first_line_intersection(eye, rangeend, false, nullptr);

  CollisionObject** resultobj = std::get_if<CollisionObject*>(&result.hit);
  return result.is_valid && resultobj && *resultobj == get_nearest_player()->get_collision_object();
}

bool
Zeekling::should_we_rebound()
{
  using RaycastResult = CollisionSystem::RaycastResult;

  Vector eye;
  const Rectf& bbox = get_bbox().grown(1.f);
  eye = bbox.get_middle();
  eye.y = bbox.get_bottom();

  const Vector rangeend = {eye.x, eye.y + CATCH_DISTANCE};

  RaycastResult result = Sector::get().get_first_line_intersection(eye, rangeend, false, nullptr);

  const Tile** resulttile = std::get_if<const Tile*>(&result.hit);
  return result.is_valid && resulttile && !(*resulttile)->is_slope();
}

void
Zeekling::active_update(float dt_sec) {
  switch (state) {
    case FLYING:
      if (!should_we_dive()) break;

      // swoop a bit up
      state = CHARGING;
      set_action("charge", m_dir);
      //m_charge_timer.start(0.1f);

      [[fallthrough]];

    case CHARGING:
    {
      if (m_sprite->animation_done())
      {
        state = DIVING;
        set_action("dive", m_dir);
        break;
      }

      //double easing_progress = static_cast<double>(m_charge_timer.get_timegone() /
      //                                             m_charge_timer.get_period());

      m_physic.set_velocity_y(-325 * QuarticEaseOut(m_sprite->get_current_frame_progress()));
      break;
    }

    case DIVING:
      if (should_we_rebound())
      {
        state = CATCH;
        m_physic.set_velocity_x(m_original_xvel);
        m_easing_progress = 0.0;
        break;
      }

      if (m_easing_progress <= 0.0)
      {
        m_original_xvel = m_physic.get_velocity_x();
        m_physic.set_velocity_x(m_original_xvel + (50 * (m_dir == Direction::LEFT ? 1 : -1)));
      }

      if (m_easing_progress >= 1.0) break;

      // swoop down
      m_easing_progress += 0.1;
      m_physic.set_velocity_y(2 * fabsf(m_original_xvel) *
                              std::min<double>(5.0, QuarticEaseIn(m_easing_progress)));

      break;

    case CATCH: {
      if (m_charge_timer.check())
      {
        state = REBOUND;
        break;
      }

      if (!m_charge_timer.started())
      {
        set_action("recover", m_dir, 1);
        m_charge_timer.start(CATCH_DURATION);
        m_original_yvel = fabsf(m_physic.get_velocity_y());
      }

      // wait for recover action
      if (m_sprite->animation_done())
      {
        set_action(m_dir);
      }

      double easing_progress = static_cast<double>(m_charge_timer.get_timegone() /
                                                   m_charge_timer.get_period());

      float ease = QuarticEaseOut(easing_progress);
      m_physic.set_velocity_y(m_original_yvel - (ease * m_original_yvel));

      break;
    }

    case REBOUND: {
      if (m_charge_timer.check())
      {
        state = RECOVERING;
        break;
      }

      if (!m_charge_timer.started())
      {
        m_charge_timer.start(CATCH_DURATION);
      }

      double easing_progress = static_cast<double>(m_charge_timer.get_timegone() /
                                                   m_charge_timer.get_period());

      float ease = QuarticEaseIn(easing_progress);
      m_physic.set_velocity_y(-fabsf(ease * m_original_yvel));

      break;
    }

    case RECOVERING: {
      // Stop recovering when we're back at initial height.
      if (get_pos().y <= m_start_position.y) {
        state = FLYING;
        m_physic.set_velocity_y(0);
        break;
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
