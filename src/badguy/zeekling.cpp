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

#include "math/easing.hpp"
#include "math/random.hpp"
#include "math/vector.hpp"
#include "math/util.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

const float FLYING_SPEED = 180.f;
const float CHARGING_SPEED = 150.f;
const float DIVING_SPEED = 280.f;

const float CHARGING_DURATION = 0.3f;
const float DIVING_DURATION = 1.5f;
const float RECOVER_DURATION = 2.8f;

Zeekling::Zeekling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/zeekling/zeekling.sprite"),
  m_catch_pos(0.f),
  m_timer(),
  m_state(FLYING)
{
  m_physic.enable_gravity(false);
  m_physic.set_velocity_x(FLYING_SPEED);
}

void Zeekling::draw(DrawingContext &context)
{
  context.color().draw_line({get_pos().x-5, m_target_y}, {get_pos().x+5, m_target_y}, Color::GREEN, 1000);
  BadGuy::draw(context);
}

void
Zeekling::initialize()
{
  m_physic.set_velocity_x(m_physic.get_velocity_x() * (m_dir == Direction::LEFT ? -1 : 1));
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
  m_physic.set_velocity_x(std::abs(m_physic.get_velocity_x()) * (m_dir == Direction::LEFT ? -1 : 1));
  std::cout << m_state << " " << m_dir << std::endl;

  switch (m_state)
  {
    case DIVING:
      // Diving attempt failed. So sad. Return to base.
      recover();
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
      recover();
      break;

    case RECOVERING:
      // I guess this is my new home now.
      fly();
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

  // Do not dive if we are too close to the player.
  float height = player->get_bbox().get_top() - get_bbox().get_top();
  if (height <= 32.f * 4.5f)
    return false;

  // Do not dive if we are too far above the player.
  if (height > 512)
    return false;

  float xdist = std::abs(eye.x - plrmid.x);
  if (!math::in_bounds(xdist, 10.f, 32.f * 15))
    return false;

  RaycastResult result = Sector::get().get_first_line_intersection(eye, plrmid, false, nullptr);

  auto* resultobj = std::get_if<CollisionObject*>(&result.hit);
  
  if (result.is_valid && resultobj &&
      *resultobj == player->get_collision_object())
  {
    m_target_y = player->get_bbox().get_top() - 10;
    return true;
  }
  else
  {
    return false;
  }
}

void Zeekling::set_speed(float speed)
{
  m_physic.set_velocity_x(speed * (m_dir == Direction::LEFT ? -1 : 1));
}

void Zeekling::fly()
{
  m_state = FLYING;
  set_speed(FLYING_SPEED);
  m_start_position.y = get_pos().y;
  set_action(m_dir);
}

void Zeekling::charge()
{
  m_state = CHARGING;
  m_timer.start(CHARGING_DURATION);
  set_speed(CHARGING_SPEED);
  set_action("charge", m_dir);
}

void Zeekling::dive()
{
  m_state = DIVING;
  m_timer.start(DIVING_DURATION);
  set_speed(DIVING_SPEED);
  set_action("dive", m_dir);
}

void Zeekling::recover()
{
  m_state = RECOVERING;
  m_catch_pos = get_pos().y;
  m_timer.start(RECOVER_DURATION);
  set_action(m_dir);
}

void
Zeekling::active_update(float dt_sec) {
  switch (m_state) {
    case FLYING:
      if (!should_we_dive())
        break;

      charge();

      break;

    case CHARGING:
      if (m_timer.check())
      {
        dive();
      }
      break;

    case DIVING:
      if (m_timer.check())
      {
        recover();
      }
      else
      {
        float dist = m_target_y - m_start_position.y;
        double progress = CubicEaseIn(static_cast<double>(1.f - m_timer.get_progress()));
        float value = m_target_y - (static_cast<float>(progress) * dist);
        Vector pos(get_pos().x, value);

        set_pos(pos);

      }
      break;

    case RECOVERING:
      if (m_timer.check())
      {
        fly();
      }
      else
      {
        float dist = m_catch_pos - m_start_position.y;
        double progress = QuadraticEaseInOut(static_cast<double>(m_timer.get_progress()));
        float value = m_catch_pos - (static_cast<float>(progress) * dist);
        Vector pos(get_pos().x, value);

        set_pos(pos);
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
