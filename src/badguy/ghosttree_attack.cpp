//  SuperTux - Ghost Tree Attacks
//  Copyright (C) 2025 Hypernoot <teratux.mail@gmail.com>
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

#include "badguy/ghosttree_attack.hpp"

#include "supertux/sector.hpp"

static const float RED_ROOT_HEIGHT = 64; //TODO calc from hitbox
static const float RED_ROOT_SPEED = 32;
static const float RED_ROOT_DELAY = 0.2;
static const float RED_ROOT_SPAN = 32;

static const float GREEN_ROOT_HEIGHT = 96;
static const float GREEN_ROOT_SPEED = 32;

static const float BLUE_ROOT_HEIGHT = 96;
static const float BLUE_ROOT_SPEED = 32;
static const float BLUE_ROOT_FIRE_DELAY = 1.0;
static const float BLUE_ROOT_FALL_DELAY = 1.0;

static const float PINCH_ROOT_HEIGHT = 96;
static const float PINCH_ROOT_SPEED = 32;
static const float PINCH_ROOT_FIRE_DELAY = 1.0;
static const float PINCH_ROOT_EXPLOSION_DELAY = 1.0;
static const float PINCH_ROOT_SPAN = 64;

// PART 1: Abstract Classes
// ----------------------------------------------------------------------------

GhostTreeAttack::GhostTreeAttack()
{
}

GhostTreeAttack::~GhostTreeAttack()
{
}

GhostTreeRoot::GhostTreeRoot(const Vector& pos, Direction dir, const std::string& sprite) :
  BadGuy(pos, dir, sprite, LAYER_OBJECTS)
{
  set_colgroup_active(COLGROUP_TOUCHABLE);
  m_physic.enable_gravity(false);
}

GhostTreeRoot::~GhostTreeRoot()
{
}

HitResponse
GhostTreeRoot::collision_badguy(BadGuy& other, const CollisionHit& hit)
{
  if (other.get_group() == COLGROUP_MOVING && other.is_snipable())
  {
    other.kill_fall();
    return ABORT_MOVE;
  }

  return BadGuy::collision_badguy(other, hit);
}

void
GhostTreeRoot::kill_fall()
{
}

// PART 2: Roots
// ----------------------------------------------------------------------------

GhostTreeRootRed::GhostTreeRootRed(const Vector& pos, GhostTreeAttack* parent) :
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/red_root.sprite"),
  m_level_bottom(pos.y),
  m_level_top(pos.y - RED_ROOT_HEIGHT),
  m_state(STATE_RISING),
  m_parent(parent)
{
  m_physic.set_velocity_y(-RED_ROOT_SPEED);
  const int variant = static_cast<int>(pos.x) % 3 + 1;
  set_action("variant" + std::to_string(variant));
}

GhostTreeRootRed::~GhostTreeRootRed()
{
}

void
GhostTreeRootRed::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  switch (m_state) {
    case STATE_RISING:
      if (get_pos().y <= m_level_top) {
        m_state = STATE_FALLING;
        m_physic.set_velocity_y(RED_ROOT_SPEED);
      }
      break;
    case STATE_FALLING:
      if (get_pos().y >= m_level_bottom) {
        m_parent->root_died();
        remove_me();
      }
      break;
    default:
      break;
  }
}

GhostTreeRootGreen::GhostTreeRootGreen(const Vector& pos, GhostTreeAttack* parent) :
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/green_root.sprite"),
  m_level_top(pos.y - GREEN_ROOT_HEIGHT),
  m_parent(parent)
{
  m_physic.set_velocity_y(-GREEN_ROOT_SPEED);
}

GhostTreeRootGreen::~GhostTreeRootGreen()
{
}

void
GhostTreeRootGreen::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  if (get_pos().y > m_level_top) {
    return;
  }
  //TODO summon explosion
  m_parent->root_died();
  remove_me();
}

GhostTreeRootBlue::GhostTreeRootBlue(const Vector& pos, GhostTreeAttack* parent) :
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/granito_root.sprite"),
  m_level_bottom(pos.y),
  m_level_top(pos.y - BLUE_ROOT_HEIGHT),
  m_state(STATE_RISING),
  m_state_timer(),
  m_parent(parent)
{
  m_physic.set_velocity_y(-BLUE_ROOT_SPEED);
}

GhostTreeRootBlue::~GhostTreeRootBlue()
{
}

void
GhostTreeRootBlue::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  switch (m_state) {
    case STATE_RISING:
      if (get_pos().y <= m_level_top) {
        m_state = STATE_FIRE_DELAY;
        m_physic.set_velocity_y(0.0);
        m_state_timer.start(BLUE_ROOT_FIRE_DELAY);
      }
      break;
    case STATE_FIRE_DELAY:
      if (m_state_timer.check()) {
        //TODO fire projectiles
        m_state = STATE_FALL_DELAY;
        m_state_timer.start(BLUE_ROOT_FALL_DELAY);
      }
      break;
    case STATE_FALL_DELAY:
      if (m_state_timer.check()) {
        m_state = STATE_FALLING;
        m_physic.set_velocity_y(BLUE_ROOT_SPEED);
      }
      break;
    case STATE_FALLING:
      if (get_pos().y >= m_level_bottom) {
        m_parent->root_died();
        remove_me();
      }
      break;
    default:
      break;
  }
}

GhostTreeRootPinch::GhostTreeRootPinch(const Vector& pos, GhostTreeAttack* parent) :
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/pinch_root.sprite"),
  m_level_top(pos.y - PINCH_ROOT_HEIGHT),
  m_state(STATE_RISING),
  m_state_timer(),
  m_parent(parent)
{
  m_physic.set_velocity_y(-PINCH_ROOT_SPEED);
}

GhostTreeRootPinch::~GhostTreeRootPinch()
{
}

void
GhostTreeRootPinch::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  switch (m_state) {
    case STATE_RISING:
      if (get_pos().y <= m_level_top) {
        m_state = STATE_FIRE_DELAY;
        m_physic.set_velocity_y(0.0);
        m_state_timer.start(PINCH_ROOT_FIRE_DELAY);
      }
      break;
    case STATE_FIRE_DELAY:
      if (m_state_timer.check()) {
        //TODO fire projectiles
        m_state = STATE_EXPLOSION_DELAY;
        m_state_timer.start(PINCH_ROOT_EXPLOSION_DELAY);
      }
      break;
    case STATE_EXPLOSION_DELAY:
      if (m_state_timer.check()) {
        //TODO summon explosion
        m_parent->root_died();
        remove_me();
      }
      break;
    default:
      break;
  }
}

// PART 3: Root Attacks
// ----------------------------------------------------------------------------

GhostTreeAttackRed::GhostTreeAttackRed(float y, float x_start, float x_end) :
m_spawn_timer(),
m_pos_y(y),
m_start_x(x_start),
m_end_x(x_end),
m_current_x(x_start),
m_remaining_roots(0),
m_ended(false)
{
  m_spawn_timer.start(0.2);
}

GhostTreeAttackRed::~GhostTreeAttackRed()
{
}

void
GhostTreeAttackRed::active_update(float dtime)
{
  if (m_ended || !m_spawn_timer.check()) {
    return;
  }

  auto& root = Sector::get().add<GhostTreeRootRed>(Vector(m_current_x, m_pos_y), this);
  m_spawn_timer.start(RED_ROOT_DELAY);
  ++m_remaining_roots;

  if (m_start_x < m_end_x) {
    m_current_x += RED_ROOT_SPAN;
    m_ended = m_current_x >= m_end_x;
  } else {
    m_current_x -= RED_ROOT_SPAN;
    m_ended = m_current_x < m_end_x;
  }
}

bool
GhostTreeAttackRed::is_done() const
{
  return m_ended && m_remaining_roots <= 0;
}

void
GhostTreeAttackRed::root_died()
{
  --m_remaining_roots;
}

GhostTreeAttackGreen::GhostTreeAttackGreen(const Vector& pos) :
m_ended(false)
{
  auto& root = Sector::get().add<GhostTreeRootGreen>(pos, this);
}

GhostTreeAttackGreen::~GhostTreeAttackGreen()
{
}

void
GhostTreeAttackGreen::active_update(float dtime)
{
  // The root updates on its own, it just notifies us when it's done.
}

bool
GhostTreeAttackGreen::is_done() const
{
  return m_ended;
}

void
GhostTreeAttackGreen::root_died()
{
  m_ended = true;
}

GhostTreeAttackBlue::GhostTreeAttackBlue(const Vector& pos) :
m_ended(false)
{
  auto& root = Sector::get().add<GhostTreeRootBlue>(pos, this);
}

GhostTreeAttackBlue::~GhostTreeAttackBlue()
{
}

void
GhostTreeAttackBlue::active_update(float dtime)
{
  // The root updates on its own, it just notifies us when it's done.
}

bool
GhostTreeAttackBlue::is_done() const
{
  return m_ended;
}

void
GhostTreeAttackBlue::root_died()
{
  m_ended = true;
}

GhostTreeAttackPinch::GhostTreeAttackPinch(const Vector& pos, float x_left, float x_right) :
m_root_ended(false),
m_left_trail(pos.y, pos.x - RED_ROOT_SPAN, x_left),
m_right_trail(pos.y, pos.x + PINCH_ROOT_SPAN, x_right)
{
  auto& root = Sector::get().add<GhostTreeRootPinch>(pos, this);
}

GhostTreeAttackPinch::~GhostTreeAttackPinch()
{
}

void
GhostTreeAttackPinch::active_update(float dtime)
{
  m_left_trail.active_update(dtime);
  m_right_trail.active_update(dtime);
}

bool
GhostTreeAttackPinch::is_done() const
{
  return m_root_ended && m_left_trail.is_done() && m_right_trail.is_done();
}

void
GhostTreeAttackPinch::root_died()
{
  m_root_ended = true;
}

