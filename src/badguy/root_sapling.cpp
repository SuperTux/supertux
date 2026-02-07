//  SuperTux - Root Sapling Badguy
//  Copyright (C) 2023 MatusGuy
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

#include "badguy/root_sapling.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/root.hpp"
#include "collision/collision_system.hpp"
#include "math/aatriangle.hpp"
#include "math/random.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"
#include "video/surface.hpp"

static const float ROOT_SAPLING_RANGE = 32.f * 20;
static const float ROOT_SAPLING_SPAWN_TIME = 1.35f;

RootSapling::RootSapling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/corrupted/root_sapling.sprite", Direction::UP,
         LAYER_TILES - 15, "images/creatures/mole/corrupted/core_glow/core_glow.sprite"),
  m_root_timer(),
  m_dead(false)
{
  m_physic.enable_gravity(false);
  set_colgroup_active(COLGROUP_MOVING);
  set_action("idle", m_dir);

  m_glowing = true;

  SoundManager::current()->preload("sounds/squish.wav");
  SoundManager::current()->preload("sounds/fall.wav");
}

void
RootSapling::kill_fall()
{
  m_dead = true;
  m_glowing = false;

  SoundManager::current()->play("sounds/fall.wav", get_pos());

  set_colgroup_active(COLGROUP_DISABLED);
  set_action("squished", m_dir);

  run_dead_script();
}

HitResponse
RootSapling::collision_badguy(BadGuy& other, const CollisionHit& hit)
{
  if (other.is_frozen())
  {
    kill_fall();
    return FORCE_MOVE;
  }

  return BadGuy::collision_badguy(other, hit);
}

bool
RootSapling::collision_squished(MovingObject& object)
{
  m_dead = true;
  m_glowing = false;

  SoundManager::current()->play("sounds/squish.wav", get_pos());

  set_action("squished", m_dir);
  set_colgroup_active(COLGROUP_DISABLED);

  auto player = dynamic_cast<Player*>(&object);
  if (player)
    player->bounce(*this);

  run_dead_script();
  return true;
}

HitResponse
RootSapling::collision_player(Player& player, const CollisionHit& hit)
{
  if (m_dir != Direction::DOWN || !hit.bottom)
    return BadGuy::collision_player(player, hit);

  player.get_physic().set_velocity_y(-player.get_physic().get_velocity_y());
  kill_fall();
  return FORCE_MOVE;
}

void
RootSapling::active_update(float dt_sec)
{
  if (m_dead) return;

  BadGuy::active_update(dt_sec);

  Player* player = get_nearest_player();
  if (!player) return;

  float distance = std::abs(get_pos().x - player->get_pos().x);
  if (distance > ROOT_SAPLING_RANGE) return;

  if (!m_root_timer.started() || m_root_timer.check())
  {
    summon_root();
    m_root_timer.start(ROOT_SAPLING_SPAWN_TIME);
  }
}

std::vector<Direction>
RootSapling::get_allowed_directions() const
{
  return { Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT };
}

float
RootSapling::get_tile_spawn_pos_offset(const Tile& tile)
{
  if (!tile.is_slope())
    return 0.f;

  AATriangle slope(tile.get_data());
  int deform = slope.dir & AATriangle::DEFORM_MASK;

  switch (m_dir)
  {
    case Direction::UP:
      if (!slope.is_south())
        return 0.f;

      if (deform == AATriangle::DEFORM_TOP)
        return 16.f;
      else
        return 32.f;

    case Direction::DOWN:
      if (slope.is_south())
        return 0.f;

      if (deform == AATriangle::DEFORM_BOTTOM)
        return -16.f;
      else
        return -32.f;

    case Direction::LEFT:
      if (!slope.is_east())
        return 0.f;

      if (deform == AATriangle::DEFORM_LEFT)
        return 16.f;
      else
        return 32.f;

    case Direction::RIGHT:
      if (slope.is_east())
        return 0.f;

      if (deform == AATriangle::DEFORM_RIGHT)
        return -16.f;
      else
        return -32.f;

    default: assert(false); break;
  }
}

void
RootSapling::summon_root()
{
  for (Player* player : Sector::get().get_players())
  {
    Vector pos;
    float* axis = nullptr;
    switch (m_dir)
    {
      case Direction::UP:
      case Direction::DOWN:
        pos.x = player->get_bbox().get_middle().x;
        axis = &pos.y;
        break;

      case Direction::LEFT:
      case Direction::RIGHT:
        pos.y = player->get_bbox().get_middle().y;
        axis = &pos.x;
        break;

      default: assert(false); break;
    }

    if (player->on_ground() && m_dir == Direction::UP)
    {
      (*axis) = player->get_bbox().get_bottom() + 1;

      bool should_summon = false;
      for (TileMap* tilemap : Sector::get().get_solid_tilemaps())
      {
        const Tile& tile = tilemap->get_tile_at(pos);
        if (tile.is_solid())
        {
          should_summon = true;
          break;
        }
      }
      if (!should_summon) continue;
    }
    else
    {
      using RaycastResult = CollisionSystem::RaycastResult;

      Vector eye, end;
      switch (m_dir)
      {
        case Direction::UP:
          eye = { player->get_bbox().get_middle().x, player->get_bbox().get_bottom() + 1 };
          end = { eye.x, eye.y + ROOT_SAPLING_RANGE };
          break;

        case Direction::DOWN:
          eye = { player->get_bbox().get_middle().x, player->get_bbox().get_top() - 1 };
          end = { eye.x, eye.y - ROOT_SAPLING_RANGE };
          break;

        case Direction::LEFT:
          eye = { player->get_bbox().get_right() + 1, player->get_bbox().get_middle().y };
          end = { eye.x + ROOT_SAPLING_RANGE, eye.y };
          break;

        case Direction::RIGHT:
          eye = { player->get_bbox().get_left() - 1, player->get_bbox().get_middle().y };
          end = { eye.x - ROOT_SAPLING_RANGE, eye.y };
          break;

        default: assert(false); break;
      }

      RaycastResult result = Sector::get().get_first_line_intersection(eye, end, false, nullptr);

      auto tile_p = std::get_if<const Tile*>(&result.hit);
      if (!tile_p || result.box.empty())
        continue;

      if ((*tile_p)->is_unisolid())
        continue;

      (*axis) = 0.f;
      switch (m_dir)
      {
        case Direction::DOWN:
          (*axis) += 32.f;
          [[fallthrough]];
        case Direction::UP:
          (*axis) += result.box.p1().y + get_tile_spawn_pos_offset(*(*tile_p));
          break;

        case Direction::RIGHT:
          (*axis) += 32.f;
          [[fallthrough]];
        case Direction::LEFT:
          (*axis) += result.box.p1().x + get_tile_spawn_pos_offset(*(*tile_p));
          break;

        default: assert(false); break;
      }

      Sizef size(32.f, 32.f);
      switch (m_dir)
      {
        case Direction::UP: size.height *= 3; break;
        case Direction::DOWN: size.height *= 3; break;
        case Direction::LEFT: size.width *= 3; break;
        case Direction::RIGHT: size.width *= 3; break;
        default: assert(false); break;
      }

      Vector bboxpos = pos;
      switch (m_dir)
      {
        case Direction::DOWN:
          bboxpos.y = std::max(bboxpos.y - (32.f * 3.f), 0.f);
          [[fallthrough]];
        case Direction::UP:
          bboxpos.x -= 16.f;
          break;

        case Direction::RIGHT:
          bboxpos.x = std::max(bboxpos.x - (32.f * 3.f), 0.f);
          [[fallthrough]];
        case Direction::LEFT:
          bboxpos.y -= 16.f;
          break;

        default: assert(false); break;
      }

      // Check if the hitbox of the root is entirely
      // occupied by solid tiles.
      Rectf space(bboxpos, size);
      if (!should_summon_root(space.grown(-1)))
        continue;

      Sector::get().add<Root>(pos, m_dir, "images/creatures/mole/corrupted/root.sprite");
    }
  }
}

bool
RootSapling::should_summon_root(const Rectf& bbox)
{
  Vector groundpos;
  switch (m_dir)
  {
    case Direction::UP:
      groundpos.x = get_x();
      groundpos.y = get_y()
                    - get_sprite()->get_current_hitbox_y_offset()
                    + get_sprite()->get_current_action_surface()->get_height();
      break;

    case Direction::DOWN:
      groundpos.x = get_x();
      groundpos.y = get_y()
                    + get_sprite()->get_current_hitbox_y_offset()
                    - get_sprite()->get_current_action_surface()->get_height()
                    - get_height();
      break;

    case Direction::RIGHT:
      groundpos.y = get_y();
      groundpos.x = get_x()
                    + get_sprite()->get_current_hitbox_x_offset()
                    - get_sprite()->get_current_action_surface()->get_width()
                    - get_width();
      break;

    case Direction::LEFT:
      groundpos.y = get_y();
      groundpos.x = get_x()
                    - get_sprite()->get_current_hitbox_x_offset()
                    + get_sprite()->get_current_action_surface()->get_width();
      break;

    default: assert(false); break;
  }

  // Check if the root spawns in the root sapling, and if so, prevent it.
  Rectf selfcheck(groundpos, get_bbox().get_size());
  if (selfcheck.grown(10.f).overlaps(bbox))
    return false;

  for (const auto& solids : Sector::get().get_solid_tilemaps())
  {
    if (solids->get_path())
      continue; // Do not support moving tilemaps. Not planned.

    // Test with all tiles in the root's hitbox
    const Rect test_tiles = solids->get_tiles_overlapping(bbox);

    for (int x = test_tiles.left; x < test_tiles.right; ++x)
    {
      for (int y = test_tiles.top; y < test_tiles.bottom; ++y)
      {
        const Tile& tile = solids->get_tile(x, y);

        if (!(tile.get_attributes() & Tile::SOLID))
          goto next_tilemap;
      }
    }

    return true;

next_tilemap:
    ;
  }

  return false;
}

void
RootSapling::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
  m_dir = invert_dir(m_dir);
}

/* EOF */
