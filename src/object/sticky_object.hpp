//  SuperTux
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

#ifndef HEADER_SUPERTUX_OBJECT_STICKY_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_STICKY_OBJECT_HPP

#include "badguy/badguy.hpp"
#include "object/moving_sprite.hpp"
#include "supertux/game_object.hpp"

class ReaderMapping;

class StickyObject : public MovingSprite
{
public:
  StickyObject(const Vector& pos,
    const std::string& sprite_name,
    int layer = LAYER_OBJECTS,
    CollisionGroup collision_group = COLGROUP_MOVING);
  StickyObject(const ReaderMapping& reader,
    const std::string& sprite_name,
    int layer = LAYER_OBJECTS,
    CollisionGroup collision_group = COLGROUP_MOVING);

  virtual void update(float dt_sec) override;
  virtual bool is_sticky() const { return m_sticky; }
  //virtual void move_for_owner(MovingObject& object);

protected:
  bool m_sticky; // determines if the object CAN stick, period. 
  bool m_sticking; // determines if the object has found something to stick to.
  //GameObject* m_owner = nullptr;

private:
  StickyObject(const StickyObject&) = delete;
  StickyObject& operator=(const StickyObject&) = delete;
};

class StickyBadguy : public BadGuy
{
public:
  StickyBadguy(const ReaderMapping& reader, const std::string& sprite_name, Direction default_direction, int layer = LAYER_OBJECTS);
  StickyBadguy(const ReaderMapping& reader, const std::string& sprite_name, int layer = LAYER_OBJECTS);

  virtual void update(float dt_sec) override;
  virtual bool is_sticky() const { return m_sticky; }
  //virtual void move_for_owner(MovingObject& object);

protected:
  bool m_sticky; // determines if the object CAN stick, period. 
  bool m_sticking; // determines if the object has found something to stick to.
  //GameObject* m_owner = nullptr;

private:
  StickyBadguy(const StickyBadguy&) = delete;
  StickyBadguy& operator=(const StickyBadguy&) = delete;
};

#endif

/* EOF */
