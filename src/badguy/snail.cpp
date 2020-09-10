//  SuperTux - Badguy "Snail"
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

#include "badguy/snail.hpp"

#include <math.h>

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

namespace {
const float SNAIL_KICK_SPEED = 500;
const int MAX_SNAIL_SQUISHES = 10;
const float SNAIL_KICK_SPEED_Y = -500; /**< y-velocity gained when kicked */
const float DANGER_SENSE_DIST = 25;
const float SHIELDED_TIME = 1.5f;
}

Snail::Snail(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/snail/snail.sprite", "left", "right"),
  state(STATE_NORMAL),
  kicked_delay_timer(),
  danger_gone_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  SoundManager::current()->preload("sounds/iceblock_bump.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
  SoundManager::current()->preload("sounds/kick.wav");
}

void
Snail::initialize()
{
  WalkingBadguy::initialize();
  be_normal();
}

void
Snail::be_normal()
{
  if (state == STATE_NORMAL) return;

  state = STATE_NORMAL;
  WalkingBadguy::initialize();
}

void
Snail::be_flat()
{
  state = STATE_FLAT;
  m_sprite->set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", 1);

  m_physic.set_velocity_x(0);
  m_physic.set_velocity_y(0);
}

void Snail::be_grabbed()
{
  state = STATE_GRABBED;
  m_sprite->set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", 1);
}

void
Snail::be_kicked(bool upwards)
{
  if(upwards)
    state = STATE_KICKED_DELAY;
  else
    state = STATE_KICKED;
  m_sprite->set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right", 1);

  m_physic.set_velocity_x(m_dir == Direction::LEFT ? -SNAIL_KICK_SPEED : SNAIL_KICK_SPEED);
  m_physic.set_velocity_y(0);

  // start a timer to delay addition of upward movement until we are (hopefully) out from under the player
  if (upwards)
    kicked_delay_timer.start(0.05f);
}

void
Snail::be_shielded()
{
  state = STATE_SHIELDED;

  m_physic.set_velocity_x(0);
  m_physic.set_velocity_y(0);

  m_sprite->set_action(m_dir == Direction::LEFT ? "shielded-left" : "shielded-right");

  danger_gone_timer.start(SHIELDED_TIME);
}

bool
Snail::can_break() const {
  return state == STATE_KICKED;
}

bool
Snail::is_in_danger()
{
  Rectf sense_zone = get_bbox().moved(Vector(0, -DANGER_SENSE_DIST));
  auto player = Sector::get().get_nearest_player(get_bbox());
  if (player && sense_zone.contains(player->get_bbox()) && player->get_velocity().y > 0)
  {
    return true;
  }
  return false;
}

void
Snail::active_update(float dt_sec)
{
  if (state == STATE_GRABBED)
    return;
  
  if (m_frozen)
  {
    BadGuy::active_update(dt_sec);
    return;
  }

  if(state == STATE_NORMAL && is_in_danger())
  {
    be_shielded();
  }

  switch (state) {

    case STATE_NORMAL:
      WalkingBadguy::active_update(dt_sec);
      return;

    case STATE_FLAT:
      if (m_sprite->animation_done()) {
        be_normal();
      }
      break;

    case STATE_KICKED_DELAY:
      if (kicked_delay_timer.check()) {
        m_physic.set_velocity_x(m_dir == Direction::LEFT ? -SNAIL_KICK_SPEED : SNAIL_KICK_SPEED);
        m_physic.set_velocity_y(SNAIL_KICK_SPEED_Y);
        state = STATE_KICKED;
      }
      break;

    case STATE_KICKED:
      m_physic.set_velocity_x(m_physic.get_velocity_x() * powf(0.99f, dt_sec/0.02f));
      if (m_sprite->animation_done() || (fabsf(m_physic.get_velocity_x()) < walk_speed)) be_normal();
      break;

    case STATE_GRABBED:
      break;

    case STATE_SHIELDED:
      if (danger_gone_timer.check())
      {
        be_normal();
      }
      break;
  }

  BadGuy::active_update(dt_sec);

  if (m_ignited)
    remove_me();
}

bool
Snail::is_freezable() const
{
  return true;
}

void
Snail::collision_solid(const CollisionHit& hit)
{
  if (m_frozen)
  {
    WalkingBadguy::collision_solid(hit);
    return;
  }

  switch (state)
  {
    case STATE_NORMAL:
    case STATE_SHIELDED:
      WalkingBadguy::collision_solid(hit);
      return;

    case STATE_KICKED:
      if (hit.left || hit.right) {
        SoundManager::current()->play("sounds/iceblock_bump.wav", get_pos());

        if ( ( m_dir == Direction::LEFT && hit.left ) || ( m_dir == Direction::RIGHT && hit.right) ){
          m_dir = (m_dir == Direction::LEFT) ? Direction::RIGHT : Direction::LEFT;
          m_sprite->set_action(m_dir == Direction::LEFT ? "flat-left" : "flat-right");

          m_physic.set_velocity_x(-m_physic.get_velocity_x());
        }
      }
      BOOST_FALLTHROUGH;
    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      if (hit.top || hit.bottom) {
        m_physic.set_velocity_y(0);
      }
      break;

    case STATE_GRABBED:
      break;
  }

  update_on_ground_flag(hit);

}

HitResponse
Snail::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (m_frozen)
    return WalkingBadguy::collision_badguy(badguy, hit);

  switch (state) {
    case STATE_NORMAL:
    case STATE_SHIELDED:
      return WalkingBadguy::collision_badguy(badguy, hit);
    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      return FORCE_MOVE;
    case STATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

HitResponse
Snail::collision_player(Player& player, const CollisionHit& hit)
{
  if (m_frozen)
    return WalkingBadguy::collision_player(player, hit);

  // handle kicks from left or right side
  if (state == STATE_FLAT && (hit.left || hit.right)) {
    if (hit.left) {
      m_dir = Direction::RIGHT;
    } else if (hit.right) {
      m_dir = Direction::LEFT;
    }
    player.kick();
    be_kicked(false);
    return FORCE_MOVE;
  }

  return BadGuy::collision_player(player, hit);
}

bool
Snail::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  Player* player = dynamic_cast<Player*>(&object);
  if (player && player->is_invincible()) {
    kill_fall();
    player->bounce(*this);
    return true;
  }

  switch (state) {

    case STATE_SHIELDED:
    case STATE_NORMAL:
      if(player && !player->m_does_buttjump)
      {
        player->bounce(*this);
        break;
      }
      BOOST_FALLTHROUGH;
    case STATE_KICKED:
      squishcount++;
      if (squishcount >= MAX_SNAIL_SQUISHES) {
        kill_fall();
        return true;
      }
      SoundManager::current()->play("sounds/stomp.wav", get_pos());
      be_flat();
      break;

    case STATE_FLAT:
      SoundManager::current()->play("sounds/kick.wav", get_pos());
      {
        MovingObject* movingobject = dynamic_cast<MovingObject*>(&object);
        if (movingobject && (movingobject->get_pos().x < get_pos().x)) {
          m_dir = Direction::RIGHT;
        } else {
          m_dir = Direction::LEFT;
        }
      }
      be_kicked(true);
      break;
    case STATE_GRABBED:
    case STATE_KICKED_DELAY:
      break;

  }

  if (player) player->bounce(*this);
  return true;
}

void
Snail::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  m_col.m_movement = pos - get_pos();
  m_dir = dir_;
  set_action(dir_ == Direction::LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
  be_grabbed();
  set_colgroup_active(COLGROUP_DISABLED);
}

void
Snail::ungrab(MovingObject& object, Direction dir_)
{
  if (dir_ == Direction::UP) {
    be_flat();
  } else {
    m_dir = dir_;
    be_kicked(true);
  }
  set_colgroup_active(COLGROUP_MOVING);
  Portable::ungrab(object, dir_);
}

bool
Snail::is_portable() const
{
  return state == STATE_FLAT && !m_ignited;
}

/* EOF */
