//  Copyright (C) 2020 Daniel Ward <weluvgoatz@gmail.com>
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

#include "object/bumper.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/badguy.hpp"
#include "badguy/crusher.hpp"
#include "object/fallblock.hpp"
#include "object/player.hpp"
#include "object/platform.hpp"
#include "object/rock.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {
const std::string TRAMPOLINE_SOUND = "sounds/trampoline.wav";
const float BOUNCE_Y = -450.0f;
const float BOUNCE_X = 700.0f;
}

Bumper::Bumper(const ReaderMapping& reader) :
  MovingSprite(reader, "images/objects/trampoline/bumper.sprite", LAYER_OBJECTS, COLGROUP_MOVING),
  m_physic(),
  m_dir(Direction::RIGHT),
  m_original_pos(),
  m_sticky()
{
  std::string dir_str;
  bool old_facing_left = false;
  m_original_pos = get_pos();

  reader.get("sticky", m_sticky, false);
  if (reader.get("direction", dir_str))
    m_dir = string_to_dir(dir_str);
  else if (reader.get("left", old_facing_left) && old_facing_left)
    m_dir = Direction::LEFT;
  set_action("normal", m_dir);
  m_physic.enable_gravity(false);
}

ObjectSettings
Bumper::get_settings()
{
  ObjectSettings result = MovingSprite::get_settings();

  result.add_bool(_("Sticky"), &m_sticky, "sticky", false);
  result.add_direction(_("Direction"), &m_dir, { Direction::RIGHT, Direction::LEFT }, "direction");
  result.reorder({"sticky", "direction", "sprite", "x", "y"});
  return result;
}

void
Bumper::update(float dt_sec)
{
  if (m_sprite->animation_done())
    set_action("normal", m_dir);

  // Pushing rocks, as well as dynamic with tilemap, platform, and fallblock.

  Rectf small_overlap_box = get_bbox().grown(1.f);

  for (auto& rock : Sector::get().get_objects_by_type<Rock>())
  {
    if (small_overlap_box.overlaps(rock.get_bbox()))
    {
      float BOUNCE_DIR = m_dir == Direction::LEFT ? -BOUNCE_X : BOUNCE_X;
      rock.get_physic().set_velocity(BOUNCE_DIR * 0.7f, BOUNCE_Y * 0.6f);
      SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
      m_sprite->set_action("swinging", m_dir, 1);
      set_pos(m_original_pos);
    }
  }

  if (m_sticky)
  {
    Rectf large_overlap_box = get_bbox().grown(8.f);

    for (auto& tm : Sector::get().get_objects_by_type<TileMap>())
    {
      if (large_overlap_box.overlaps(tm.get_bbox()) && tm.is_solid() && glm::length(tm.get_movement(true)) > (1.f * dt_sec) &&
        !Sector::get().is_free_of_statics(large_overlap_box))
      {
        m_col.set_movement(tm.get_movement(true));
        m_original_pos = get_pos();
        return;
      }
    }

    for (auto& platform : Sector::get().get_objects_by_type<Platform>())
    {
      if (large_overlap_box.overlaps(platform.get_bbox()))
      {
        m_col.set_movement(platform.get_movement());
        m_original_pos = get_pos();
        return;
      }
    }

    for (auto& fallblock : Sector::get().get_objects_by_type<FallBlock>())
    {
      if (small_overlap_box.overlaps(fallblock.get_bbox()))
      {
        m_col.set_movement((fallblock.get_state() == FallBlock::State::LAND) ? Vector(0.f, 0.f) : fallblock.get_physic().get_movement(dt_sec));
        m_original_pos = get_pos();
        return;
      }
    }
  }

  m_col.set_movement(m_physic.get_movement (dt_sec));
}

HitResponse
Bumper::collision(GameObject& other, const CollisionHit& hit)
{
  auto player = dynamic_cast<Player*> (&other);
  if (player)
  {
    float BOUNCE_DIR = m_dir == Direction::LEFT ? -BOUNCE_X : BOUNCE_X;
    player->get_physic().set_velocity(0.f, player->is_swimming() ? 0.f : BOUNCE_Y);
    player->sideways_push(BOUNCE_DIR);
    SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
    set_action("swinging", m_dir, 1);
  }

  auto badguy = dynamic_cast<BadGuy*> (&other);
  if (badguy)
  {
    float BOUNCE_DIR = (m_dir == Direction::LEFT) ? -400.f : 400.f;
    badguy->get_physic().set_velocity(BOUNCE_DIR, 0.f);
    SoundManager::current()->play(TRAMPOLINE_SOUND, get_pos());
    set_action("swinging", m_dir, 1);
  }

  auto bumper = dynamic_cast<Bumper*> (&other);
  if (bumper)
  {
    m_physic.set_velocity_y(0);
    return FORCE_MOVE;
  }
  return ABORT_MOVE;
}

Physic&
Bumper::get_physic()
{
  return m_physic;
}

void
Bumper::after_editor_set()
{
  MovingSprite::after_editor_set();

  set_action("normal", m_dir);
}

void
Bumper::on_flip(float height)
{
  MovingSprite::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
