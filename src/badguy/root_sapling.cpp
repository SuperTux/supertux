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
#include "math/random.hpp"
#include "object/player.hpp"
#include "object/tilemap.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/flip_level_transformer.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const float ROOT_SAPLING_RANGE = 32.f*20;
static const float ROOT_SAPLING_SPAWN_TIME = 2.f;

RootSapling::RootSapling(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/mole/corrupted/root_sapling.sprite", Direction::UP,
         LAYER_TILES-10, "images/creatures/mole/corrupted/core_glow/core_glow.sprite"),
  m_root_timer(),
  m_dead(false)
{
  m_physic.enable_gravity(false);
  set_colgroup_active(COLGROUP_STATIC);

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
  set_action("squished");

  run_dead_script();
}

HitResponse
RootSapling::collision_badguy(BadGuy&, const CollisionHit&)
{
  return FORCE_MOVE;
}

bool
RootSapling::collision_squished(GameObject& object)
{
  m_dead = true;
  m_glowing = false;

  SoundManager::current()->play("sounds/squish.wav", get_pos());

  set_action("squished");
  set_colgroup_active(COLGROUP_DISABLED);

  auto player = dynamic_cast<Player*>(&object);
  if (player) {
    player->bounce(*this);
  }

  run_dead_script();
  return true;
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

void
RootSapling::summon_root()
{
  Player* player = get_nearest_player();
  if (!player) return;

  Vector pos;
  float* axis;
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
  }

  if (player->on_ground())
  {
    (*axis) = player->get_bbox().get_bottom() + 1;

    bool should_summon = false;
    for (TileMap* map : Sector::get().get_solid_tilemaps())
    {
      const Tile& tile = map->get_tile_at(pos);
      if (tile.is_solid())
      {
        should_summon = true;
        break;
      }
    }
    if (!should_summon) return;
  }
  else
  {
    using RaycastResult = CollisionSystem::RaycastResult;

    Vector eye, end;
    switch (m_dir)
    {
      case Direction::UP:
        eye = {player->get_bbox().get_middle().x, player->get_bbox().get_bottom() + 1};
        end = {eye.x, eye.y + 600};
        break;

      case Direction::DOWN:
        eye = {player->get_bbox().get_middle().x, player->get_bbox().get_top() - 1};
        end = {eye.x, eye.y - 600};
        break;

      case Direction::LEFT:
        eye = {player->get_bbox().get_right() + 1, player->get_bbox().get_middle().y};
        end = {eye.x + 600, eye.y};
        break;

      case Direction::RIGHT:
        eye = {player->get_bbox().get_left() - 1, player->get_bbox().get_middle().y};
        end = {eye.x - 600, eye.y};
        break;
    }

    RaycastResult result = Sector::get().get_first_line_intersection(eye, end, true, nullptr);

    if (std::holds_alternative<const Tile*>(result.hit) && !result.box.empty())
    {
      switch (m_dir)
      {
        case Direction::UP:
        case Direction::DOWN:
          (*axis) = result.box.p1().y;
          break;

        case Direction::LEFT:
        case Direction::RIGHT:
          (*axis) = result.box.p1().x;
          break;
      }
    }
    else
    {
      return;
    }
  }

  Sizef size(32.f, 32.f);
  switch (m_dir)
  {
    case Direction::UP: size.height *= 3; break;
    case Direction::DOWN: size.height *= -2; break;
    case Direction::LEFT: size.width *= 3; break;
    case Direction::RIGHT: size.width *= -2; break;
  }

  Rectf space(pos, size);
  if (!Sector::get().is_free_of_tiles(space, true, 0))
    return;

  Sector::get().add<Root>(pos, m_dir, "images/creatures/mole/corrupted/root.sprite");
}

void
RootSapling::on_flip(float height)
{
  BadGuy::on_flip(height);
  FlipLevelTransformer::transform_flip(m_flip);
}

/* EOF */
