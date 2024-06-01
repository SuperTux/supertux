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

#include "object/sticky_object.hpp"

#include "badguy/badguy.hpp"
#include "badguy/crusher.hpp"
#include "object/fallblock.hpp"
#include "object/moving_sprite.hpp"
#include "object/platform.hpp"
#include "object/tilemap.hpp"
#include "supertux/sector.hpp"

StickyObject::StickyObject(const Vector& pos, const std::string& sprite_name,
                           int layer, CollisionGroup collision_group) :
  MovingSprite(pos, sprite_name, layer, collision_group),
  m_sticky(),
  m_sticking(),
  m_displacement_from_owner()
{
}

StickyObject::StickyObject(const ReaderMapping& reader, const std::string& sprite_name, int layer, CollisionGroup collision_group) :
  MovingSprite(reader, sprite_name, layer, collision_group),
  m_sticky(),
  m_sticking(),
  m_displacement_from_owner()
{
}

void
StickyObject::update(float dt_sec)
{
  Rectf large_overlap_box = get_bbox().grown(8.f);

  for (auto& tm : Sector::get().get_objects_by_type<TileMap>())
  {
    if (large_overlap_box.overlaps(tm.get_bbox()) && tm.is_solid() && glm::length(tm.get_movement(true)) > (1.f * dt_sec) &&
      !Sector::get().is_free_of_statics(large_overlap_box))
    {
      m_col.set_movement(tm.get_movement(true));
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - tm.get_bbox().p1();
        m_sticking = true;
      }
      m_col.set_pos(tm.get_bbox().p1() + m_displacement_from_owner);
      return;
    }
  }

  for (auto& crusher : Sector::get().get_objects_by_type<Crusher>())
  {
    if (large_overlap_box.overlaps(crusher.get_bbox()))
    {
      m_col.set_movement(crusher.get_movement());
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - crusher.get_pos();
        m_sticking = true;
      }
      move_for_owner(crusher);
      return;
    }
  }

  for (auto& platform : Sector::get().get_objects_by_type<Platform>())
  {
    if (large_overlap_box.overlaps(platform.get_bbox()))
    {
      m_col.set_movement(platform.get_movement());
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - platform.get_pos();
        m_sticking = true;
      }
      move_for_owner(platform);
      return;
    }
  }

  for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>())
  {
    if (large_overlap_box.overlaps(fallblock.get_bbox()))
    {
      m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - fallblock.get_pos();
        m_sticking = true;
      }
      move_for_owner(fallblock);
      return;
    }
  }

}

ObjectSettings
StickyObject::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();
  result.add_bool(_("Sticky"), &m_sticky, "sticky", false);

  result.reorder({"sticky", "sprite", "z-pos", "x", "y" });

  return result;
}

void
StickyObject::move_for_owner(MovingObject& object)
{
  m_col.set_pos(object.get_pos() + m_displacement_from_owner);
}

StickyBadguy::StickyBadguy(const ReaderMapping& mapping, const std::string& sprite_name, Direction default_direction, int layer, CollisionGroup collision_group) :
  BadGuy(mapping, sprite_name, default_direction, layer),
  m_sticky(),
  m_sticking(),
  m_displacement_from_owner()
{
  set_group(collision_group);
}

StickyBadguy::StickyBadguy(const ReaderMapping& mapping, const std::string& sprite_name, int layer, CollisionGroup collision_group) :
  BadGuy(mapping, sprite_name, layer),
  m_sticky(),
  m_sticking(),
  m_displacement_from_owner()
{
  set_group(collision_group);
}

void
StickyBadguy::sticky_update(float dt_sec)
{
  Rectf large_overlap_box = get_bbox().grown(8.f);

  for (auto& tm : Sector::get().get_objects_by_type<TileMap>())
  {
    if (large_overlap_box.overlaps(tm.get_bbox()) && tm.is_solid() && glm::length(tm.get_movement(true)) > (1.f * dt_sec) &&
      !Sector::get().is_free_of_statics(large_overlap_box))
    {
      m_col.set_movement(tm.get_movement(true));
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - tm.get_bbox().p1();
        m_sticking = true;
      }
      m_col.set_pos(tm.get_bbox().p1() + m_displacement_from_owner);
      return;
    }
  }

  for (auto& crusher : Sector::get().get_objects_by_type<Crusher>())
  {
    if (large_overlap_box.overlaps(crusher.get_bbox()))
    {
      m_col.set_movement(crusher.get_movement());
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - crusher.get_pos();
        m_sticking = true;
      }
      move_for_owner(crusher);
      return;
    }
  }

  for (auto& platform : Sector::get().get_objects_by_type<Platform>())
  {
    if (large_overlap_box.overlaps(platform.get_bbox()))
    {
      m_col.set_movement(platform.get_movement());
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - platform.get_pos();
        m_sticking = true;
      }
      move_for_owner(platform);
      return;
    }
  }

  for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>())
  {
    if (large_overlap_box.overlaps(fallblock.get_bbox()))
    {
      m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
      if (!m_sticking)
      {
        m_displacement_from_owner = get_pos() - fallblock.get_pos();
        m_sticking = true;
      }
      move_for_owner(fallblock);
      return;
    }
  }
}

ObjectSettings
StickyBadguy::get_settings()
{
  ObjectSettings result = BadGuy::get_settings();
  result.add_bool(_("Sticky"), &m_sticky, "sticky", false);

  result.reorder({ "sticky", "direction", "sprite", "z-pos", "x", "y" });

  return result;
}

void
StickyBadguy::move_for_owner(MovingObject& object)
{
  m_col.set_pos(object.get_pos() + m_displacement_from_owner);
}

/* EOF */
