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
#include "object/fallblock.hpp"
#include "object/moving_sprite.hpp"
#include "object/platform.hpp"
#include "object/tilemap.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

StickyObject::StickyObject(const Vector& pos, const std::string& sprite_name_,
  int layer_, CollisionGroup collision_group) :
  MovingSprite(pos, sprite_name_, layer_, collision_group),
  m_sticky(),
  m_sticking()
  //m_owner(nullptr)
{
}

StickyObject::StickyObject(const ReaderMapping& reader, const std::string& sprite_name_, int layer_, CollisionGroup collision_group) :
  MovingSprite(reader, sprite_name_, layer_, collision_group),
  m_sticky(),
  m_sticking()
  //m_owner(nullptr)
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
      m_sticking = true;
      return;
    }
  }

  for (auto& platform : Sector::get().get_objects_by_type<Platform>())
  {
    if (large_overlap_box.overlaps(platform.get_bbox()))
    {
      m_col.set_movement(platform.get_movement());
      m_sticking = true;
      return;
    }
  }

  for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>())
  {
    if (large_overlap_box.overlaps(fallblock.get_bbox()))
    {
      m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
      m_sticking = true;
      return;
    }
  }

}

/*void
StickyObject::move_for_owner(MovingObject& object) {
  if (m_owner == nullptr) {
    return;
  }
  m_col.set_pos(object.get_pos());
}*/

StickyBadguy::StickyBadguy(const ReaderMapping& mapping, const std::string& sprite_name_, Direction default_direction_, int layer_, CollisionGroup collision_group) :
  BadGuy(mapping, sprite_name_, default_direction_, layer_),
  m_sticky(),
  m_sticking()
{
  set_group(collision_group);
}

StickyBadguy::StickyBadguy(const ReaderMapping& mapping, const std::string& sprite_name_, int layer_, CollisionGroup collision_group) :
  BadGuy(mapping, sprite_name_, layer_),
  m_sticky(),
  m_sticking()
{
  set_group(collision_group);
}

void StickyBadguy::active_update(float dt_sec) {
  Rectf large_overlap_box = get_bbox().grown(8.f);

  for (auto& tm : Sector::get().get_objects_by_type<TileMap>())
  {
    if (large_overlap_box.overlaps(tm.get_bbox()) && tm.is_solid() && glm::length(tm.get_movement(true)) > (1.f * dt_sec) &&
      !Sector::get().is_free_of_statics(large_overlap_box))
    {
      m_col.set_movement(tm.get_movement(true));
      m_sticking = true;
      return;
    }
  }

  for (auto& platform : Sector::get().get_objects_by_type<Platform>())
  {
    if (large_overlap_box.overlaps(platform.get_bbox()))
    {
      m_col.set_movement(platform.get_movement());
      m_sticking = true;
      return;
    }
  }

  for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>())
  {
    if (large_overlap_box.overlaps(fallblock.get_bbox()))
    {
      m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
      m_sticking = true;
      return;
    }
  }
}

/* EOF */
