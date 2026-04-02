//  SuperTux
//  Copyright (C) 2026 e
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

#include "badguy/granito_sentinel.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "object/shard.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

static const std::string CORRUPTED_GRANITO_SOUND = "sounds/hop.ogg";
static const float HORIZONTAL_SPEED = 220;
static const float VERTICAL_SPEED = -450;
static const std::string SHARD_SPRITE = "images/creatures/granito/corrupted/big/root_spike.sprite";

GranitoSentinel::GranitoSentinel(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/granito/corrupted/corrupted_granito.sprite"),
  recover_timer(),
  state()
{
  parse_type(reader);

  SoundManager::current()->preload(CORRUPTED_GRANITO_SOUND);
  set_state(STANDING);
}

GranitoSentinel::GranitoSentinel(const ReaderMapping& reader, int type) :
  GranitoSentinel(reader)
{
}

void
GranitoSentinel::set_state(GranitoSentinelState newState)
{
  if (newState == STANDING)
  {
    m_physic.set_velocity(0, 0);
    set_action("standing", m_dir);

    recover_timer.start(0.5);
  }
  else
  {
    if (newState == CHARGING)
    {
      set_action("charging", m_dir, 1);
    }
    else if (newState == JUMPING)
    {
      set_action("jumping", m_dir);
      m_physic.set_velocity_x(m_dir == Direction::LEFT ? -HORIZONTAL_SPEED : HORIZONTAL_SPEED);
      m_physic.set_velocity_y(VERTICAL_SPEED);
      SoundManager::current()->play( CORRUPTED_GRANITO_SOUND, get_pos());
    }
  }

  state = newState;
}

bool
GranitoSentinel::collision_squished(MovingObject& object)
{
  if (m_frozen)
    return BadGuy::collision_squished(object);

  set_action("squished", m_dir);
  kill_squished(object);
  return true;
}

void
GranitoSentinel::collision_solid(const CollisionHit& hit)
{
  if (m_frozen || BadGuy::get_state() == STATE_BURNING)
  {
    BadGuy::collision_solid(hit);
    return;
  }

  // Default behaviour (i.e. stop at floor/walls) when squished.
  if (BadGuy::get_state() == STATE_SQUISHED)
    BadGuy::collision_solid(hit);

  // Ignore collisions while standing still.
  if (state != JUMPING)
    return;

  // Check if we hit the floor while falling.
  if (hit.bottom && m_physic.get_velocity_y() > 0)
    {
    set_state(STANDING);
    
    run_dead_script();

    Sector::get().add<Shard>(get_bbox().get_middle(), Vector(100.f, -500.f), SHARD_SPRITE);
    Sector::get().add<Shard>(get_bbox().get_middle(), Vector(270.f, -350.f), SHARD_SPRITE);
    Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-100.f, -500.f),SHARD_SPRITE);
    Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-270.f, -350.f),SHARD_SPRITE);

    remove_me();
    }

  // Check if we hit the roof while climbing.
  if (hit.top)
    m_physic.set_velocity_y(0);

  // Check if we hit left or right while moving in either direction.
  if (hit.left || hit.right)
  {
    m_dir = m_dir == Direction::LEFT ? Direction::RIGHT : Direction::LEFT;
    set_action("jumping", m_dir);
    m_physic.set_velocity_x(-0.25f*m_physic.get_velocity_x());
  }
}

HitResponse
GranitoSentinel::collision_badguy(BadGuy& , const CollisionHit& hit)
{
  // Behaviour for badguy collisions is the same as for collisions with solids.
  collision_solid(hit);

  return CONTINUE;
}

void
GranitoSentinel::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);

  // No change if frozen.
  if (m_frozen)
    return;

  // Charge when fully recovered.
  if (state == STANDING && should_jump())
  {
    set_state(CHARGING);
    return;
  }

  // Jump as soon as charging animation completed.
  if (state == CHARGING && m_sprite->animation_done())
  {
    set_state(JUMPING);
    return;
  }
}

bool
GranitoSentinel::should_jump()
{
  if (m_frozen)
    return false;

  Player* player = get_nearest_player();
  if (!player)
    return false;

  float x_diff = std::abs(this->get_x() - player->get_x());

  Rectf check_box = get_bbox();
        check_box.set_left(player->get_x());
        check_box.set_right(player->get_x() + 32.f);
        check_box.set_top(this->get_y());
        check_box.set_bottom(player->get_y());

  bool tux_is_covered = !Sector::get().is_free_of_statics(check_box);

  float height = player->get_y() - this->get_y();
  if (height <= 0.f)
    return false;

  if (height > 3200.f)
    return false;

  bool facing_tux = (m_dir == Direction::LEFT ? player->get_x() < this->get_x() : player->get_x() > this->get_x());

  float parabola_point = find_parabola_point(x_diff);

  bool tux_on_parabola = 100.f >= std::abs(player->get_y() - parabola_point);

  if (facing_tux && !tux_is_covered && tux_on_parabola)
    return true;

  return false;
}

float
GranitoSentinel::find_parabola_point(float x_value)
{
  float g = Sector::get().get_gravity() * 100.f;
  float d = 0.015f;

  float a = (g/(2*(HORIZONTAL_SPEED*HORIZONTAL_SPEED)));
  float b = ((VERTICAL_SPEED/HORIZONTAL_SPEED)+((g*d)/(2*HORIZONTAL_SPEED)));

  return a*(x_value*x_value) + b*x_value + get_y();
}

void
GranitoSentinel::unfreeze(bool melt)
{
  BadGuy::unfreeze(melt);
  initialize();
}

bool
GranitoSentinel::is_freezable() const
{
  return false;
}

