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

#include "badguy/mriceblock.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

namespace {
const float KICKSPEED = 500;
const int MAXSQUISHES = 10;
const float NOKICK_TIME = 0.1f;
}

MrIceBlock::MrIceBlock(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/mr_iceblock/mr_iceblock.sprite", "left", "right"),
  ice_state(ICESTATE_NORMAL),
  nokick_timer(),
  flat_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  SoundManager::current()->preload("sounds/iceblock_bump.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
  SoundManager::current()->preload("sounds/kick.wav");
}

void
MrIceBlock::initialize()
{
  WalkingBadguy::initialize();
  set_state(ICESTATE_NORMAL);
}

void
MrIceBlock::active_update(float dt_sec)
{
  if (ice_state == ICESTATE_GRABBED)
    return;

  if (ice_state == ICESTATE_FLAT && flat_timer.check()) {
    set_state(ICESTATE_WAKING);
  }

  if (ice_state == ICESTATE_WAKING && m_sprite->animation_done()) {
    set_state(ICESTATE_NORMAL);
  }

  if (ice_state == ICESTATE_NORMAL)
  {
    WalkingBadguy::active_update(dt_sec);
    return;
  }

  BadGuy::active_update(dt_sec);
}

bool
MrIceBlock::can_break() const {
  return ice_state == ICESTATE_KICKED;
}

void
MrIceBlock::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);

  if (hit.top || hit.bottom) { // floor or roof
    m_physic.set_velocity_y(0);
  }

  // hit left or right
  switch (ice_state) {
    case ICESTATE_NORMAL:
      WalkingBadguy::collision_solid(hit);
      break;
    case ICESTATE_KICKED: {
      if ((hit.right && m_dir == Direction::RIGHT) || (hit.left && m_dir == Direction::LEFT)) {
        m_dir = (m_dir == Direction::LEFT) ? Direction::RIGHT : Direction::LEFT;
        SoundManager::current()->play("sounds/iceblock_bump.wav", get_pos());
        m_physic.set_velocity_x(-m_physic.get_velocity_x()*.975f);
      }
      set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      if (fabsf(m_physic.get_velocity_x()) < walk_speed * 1.5f)
        set_state(ICESTATE_NORMAL);
      break;
    }
    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
      m_physic.set_velocity_x(0);
      break;
    case ICESTATE_GRABBED:
      break;
  }
}

HitResponse
MrIceBlock::collision(GameObject& object, const CollisionHit& hit)
{
  if (ice_state == ICESTATE_GRABBED)
    return FORCE_MOVE;

  return BadGuy::collision(object, hit);
}

HitResponse
MrIceBlock::collision_player(Player& player, const CollisionHit& hit)
{
  // handle kicks from left or right side
  if ((ice_state == ICESTATE_WAKING || ice_state == ICESTATE_FLAT) && get_state() == STATE_ACTIVE) {
    if (hit.left) {
      m_dir = Direction::RIGHT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    } else if (hit.right) {
      m_dir = Direction::LEFT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    }
  }

  return BadGuy::collision_player(player, hit);
}

HitResponse
MrIceBlock::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch (ice_state) {
    case ICESTATE_NORMAL:
      return WalkingBadguy::collision_badguy(badguy, hit);
    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
      return FORCE_MOVE;
    case ICESTATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

bool
MrIceBlock::collision_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
  if (player && (player->m_does_buttjump || player->is_invincible())) {
    player->bounce(*this);
    kill_fall();
    return true;
  }

  switch (ice_state)
  {
    case ICESTATE_KICKED:
      {
        auto badguy = dynamic_cast<BadGuy*>(&object);
        if (badguy) {
          badguy->kill_fall();
          break;
        }
      }
      BOOST_FALLTHROUGH;

    case ICESTATE_NORMAL:
      {
        squishcount++;
        if (squishcount >= MAXSQUISHES) {
          kill_fall();
          return true;
        }
      }

      SoundManager::current()->play("sounds/stomp.wav", get_pos());
      m_physic.set_velocity_x(0);
      m_physic.set_velocity_y(0);
      set_state(ICESTATE_FLAT);
      nokick_timer.start(NOKICK_TIME);
      break;

    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
      {
        auto movingobject = dynamic_cast<MovingObject*>(&object);
        if (movingobject && (movingobject->get_pos().x < get_pos().x)) {
          m_dir = Direction::RIGHT;
        } else {
          m_dir = Direction::LEFT;
        }
      }
      if (nokick_timer.check()) set_state(ICESTATE_KICKED);
      break;

    case ICESTATE_GRABBED:
      assert(false);
      break;
  }

  if (player) player->bounce(*this);
  return true;
}

void
MrIceBlock::set_state(IceState state_)
{
  if (ice_state == state_)
    return;

  switch (state_) {
    case ICESTATE_NORMAL:
      set_action(m_dir == Direction::LEFT ? "left" : "right", /* loops = */ -1);
      WalkingBadguy::initialize();
      break;
    case ICESTATE_FLAT:
      set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      flat_timer.start(4);
      break;
    case ICESTATE_KICKED:
      SoundManager::current()->play("sounds/kick.wav", get_pos());

      m_physic.set_velocity_x(m_dir == Direction::LEFT ? -KICKSPEED : KICKSPEED);
      set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      // we should slide above 1 block holes now...
      m_col.m_bbox.set_size(34, 31.8f);
      break;
    case ICESTATE_GRABBED:
      flat_timer.stop();
      break;
    case ICESTATE_WAKING:
      m_sprite->set_action(m_dir == Direction::LEFT ? "waking-left" : "waking-right",
                         /* loops = */ 1);
      break;
    default:
      assert(false);
  }
  ice_state = state_;
}

void
MrIceBlock::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  m_col.m_movement = pos - get_pos();
  m_dir = dir_;
  set_action(dir_ == Direction::LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
  set_state(ICESTATE_GRABBED);
  set_colgroup_active(COLGROUP_DISABLED);
}

void
MrIceBlock::ungrab(MovingObject& object, Direction dir_)
{
  if (dir_ == Direction::UP) {
    m_physic.set_velocity_y(-KICKSPEED);
    set_state(ICESTATE_FLAT);
  } else if (dir_ == Direction::DOWN) {
    Vector mov(0, 32);
    if (Sector::get().is_free_of_statics(get_bbox().moved(mov), this)) {
      // There is free space, so throw it down
      SoundManager::current()->play("sounds/kick.wav", get_pos());
      m_physic.set_velocity_y(KICKSPEED);
    }
    set_state(ICESTATE_FLAT);
  } else {
    m_dir = dir_;
    set_state(ICESTATE_KICKED);
  }
  set_colgroup_active(COLGROUP_MOVING);
  Portable::ungrab(object, dir_);
}

bool
MrIceBlock::is_portable() const
{
  return ice_state == ICESTATE_FLAT;
}

void
MrIceBlock::ignite() {
  set_state(ICESTATE_NORMAL);
  BadGuy::ignite();
}

/* EOF */
