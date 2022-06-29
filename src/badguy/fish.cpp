//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "badguy/fish.hpp"

#include "sprite/sprite.hpp"
#include "supertux/tile.hpp"

static const float FISH_JUMP_POWER = -600;
static const float FISH_WAIT_TIME = 1;

Fish::Fish(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/fish/fish.sprite", LAYER_TILES-1),
  waiting(),
  stop_y(0)
{
  m_physic.enable_gravity(true);
}

void
Fish::collision_solid(const CollisionHit& chit)
{
  hit(chit);
  if (m_frozen)
    BadGuy::collision_solid(chit);
}

HitResponse
Fish::collision_badguy(BadGuy& , const CollisionHit& chit)
{
  return hit(chit);
}

void
Fish::draw(DrawingContext& context)
{
  if (waiting.started())
    return;

  BadGuy::draw(context);
}

HitResponse
Fish::hit(const CollisionHit& hit_)
{
  if (hit_.top) {
    m_physic.set_velocity_y(0);
  }

  return CONTINUE;
}

void
Fish::collision_tile(uint32_t tile_attributes)
{
  if ((tile_attributes & Tile::WATER) && (m_physic.get_velocity_y() >= 0)) {

    // initialize stop position if uninitialized
    if (stop_y == 0) stop_y = get_pos().y + m_col.m_bbox.get_height();

    // stop when we have reached the stop position
    if (get_pos().y >= stop_y) {
      if (!m_frozen)
        start_waiting();
      m_col.set_movement(Vector(0, 0));
    }

  }
  if ((!(tile_attributes & Tile::WATER) || m_frozen) && (tile_attributes & Tile::HURTS)) {
    kill_fall();
  }
}

void
Fish::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  // waited long enough?
  if (waiting.check()) {
    jump();
  }

  // set sprite
  if (!m_frozen)
    m_sprite->set_action(m_physic.get_velocity_y() < 0 ? "normal" : "down");

  // we can't afford flying out of the tilemap, 'cause the engine would remove us.
  if ((get_pos().y - 31.8f) < 0) // too high, let us fall
  {
    m_physic.set_velocity_y(0);
    m_physic.enable_gravity(true);
  }

  if (m_ignited)
    remove_me();
}

void
Fish::start_waiting()
{
  waiting.start(FISH_WAIT_TIME);
  set_colgroup_active(COLGROUP_DISABLED);
  m_physic.enable_gravity(false);
  m_physic.set_velocity_y(0);
}

void
Fish::jump()
{
  m_physic.set_velocity_y(FISH_JUMP_POWER);
  m_physic.enable_gravity(true);
  set_colgroup_active(COLGROUP_MOVING);
}

void
Fish::freeze()
{
  BadGuy::freeze();
  m_sprite->set_action(m_physic.get_velocity_y() < 0 ? "iced" : "iced-down");
  m_sprite->set_color(Color(1.0f, 1.0f, 1.0f));
  waiting.stop();
}

void
Fish::unfreeze(bool melt)
{ // does this happen at all? (or do fishes die when they fall frozen?)
  BadGuy::unfreeze(melt);
  start_waiting();
}

void
Fish::kill_fall()
{
  m_sprite->set_action("normal");
  BadGuy::kill_fall();
}

bool
Fish::is_freezable() const
{
  return true;
}

/* EOF */
