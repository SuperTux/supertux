//  Copyright (C) 2024 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/bigsnowball.hpp"

#include "badguy/yeti.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
  const float SPEED_X = 350.f;
  const float SPEED_Y = -300.f;
}

BigSnowball::BigSnowball(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/big_snowball/big_snowball.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_physic(),
  m_dir(Direction::LEFT),
  m_speed()
{
  reader.get("speed", m_speed, SPEED_X);
  m_physic.set_velocity_x(m_dir == Direction::RIGHT ? m_speed : -m_speed);
  std::string dir_str;
  if (reader.get("direction", dir_str)) {
    m_dir = string_to_dir(dir_str);
  }
  set_action("normal", m_dir);
}

BigSnowball::BigSnowball(const Vector& pos, const Direction& dir, bool bounce) :
  MovingSprite(pos, "images/objects/big_snowball/big_snowball.sprite", LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  m_physic(),
  m_dir(Direction::LEFT),
  m_speed()
{
  m_dir = dir;
  m_speed = SPEED_X;
  m_physic.set_velocity_x(m_dir == Direction::RIGHT ? SPEED_X : -SPEED_X);
  if (bounce) {
    m_physic.set_velocity_y(SPEED_Y);
  }
  set_action("normal", m_dir);
}

ObjectSettings
BigSnowball::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_float(_("Speed"), &m_speed, "speed", SPEED_X);
  result.add_direction(_("Direction"), &m_dir, { Direction::RIGHT, Direction::LEFT }, "direction");
  result.reorder({ "direction", "sprite", "x", "y" });
  return result;
}

void
BigSnowball::update(float dt_sec)
{
  if (get_bbox().get_top() > Sector::get().get_height()) {
    remove_me();
  }

  // Left-right faux collision

  Rectf side_look_box = get_bbox().grown(-1.f);
  side_look_box.set_left(get_bbox().get_left() + (m_dir == Direction::LEFT ? -1.f : 1.f));
  side_look_box.set_right(get_bbox().get_right() + (m_dir == Direction::LEFT ? -1.f : 1.f));
  if (!Sector::get().is_free_of_statics(side_look_box))
  {
    m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
    set_action(m_dir);
    m_physic.set_velocity_x(-m_physic.get_velocity_x());
  }

  Vector movement = m_physic.get_movement(dt_sec);
  m_sprite->set_angle(m_sprite->get_angle() + movement.x * 3.141592653898f / 2.f);
  m_col.set_movement(movement);
  m_col.propagate_movement(movement);
}

HitResponse
BigSnowball::collision(GameObject& other, const CollisionHit& hit)
{
  // ignore collisions with yeti
  auto* yeti = dynamic_cast<Yeti*>(&other);
  if (yeti) {
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
BigSnowball::collision_solid(const CollisionHit& hit)
{
  if (hit.bottom) {
    m_physic.set_velocity_y(0.f);
  }
  if (m_dir == Direction::LEFT && hit.left)
  {
    m_physic.set_velocity_x(m_speed);
    m_dir = Direction::RIGHT;
  }
  if (m_dir == Direction::RIGHT && hit.right)
  {
    m_physic.set_velocity_x(-m_speed);
    m_dir = Direction::LEFT;
  }
}

/* EOF */
