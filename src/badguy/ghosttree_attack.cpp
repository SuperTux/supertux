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

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "badguy/dart.hpp"
#include "math/random.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "object/shard.hpp"
#include "object/sprite_particle.hpp"
#include "supertux/sector.hpp"
#include "video/surface.hpp"

static const unsigned MAIN_ROOT_COUNT = 3;
static const float MAIN_ROOT_DELAY = 1.5f;
static const float MAIN_ROOT_HILL_OFFSET = 8.f;
static const float MAIN_ROOT_HATCH_OFFSET = 16.f;
static const float MAIN_ROOT_HATCH_DURATION = .3f;
static const float MAIN_ROOT_RISE_DURATION = .9f;
static const float MAIN_ROOT_FALL_DURATION = 1.3f;
static const float MAIN_ROOT_SOUND_PITCH = 0.85f; // Pitch multiplier

static const float RED_ROOT_SPEED = 216;
static const float RED_ROOT_DELAY = 0.15f;
static const float RED_ROOT_SPAN = 32;

static const float GREEN_ROOT_SPEED = 64;
static const std::string GREEN_ROOT_SHARD_SPRITE = "images/creatures/granito/corrupted/big/root_spike.sprite";

static const float BLUE_ROOT_SPEED = 64;
static const float BLUE_ROOT_FIRE_DELAY = 1.0;
static const float BLUE_ROOT_FALL_DELAY = 1.0;
static const std::string BLUE_ROOT_PROJECTILE = "images/creatures/ghosttree/blue_root.sprite";

static const float PINCH_ROOT_SPEED = 64;
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

GhostTreeRoot::GhostTreeRoot(const Vector& pos, Direction dir, const std::string& sprite):
  BadGuy(pos, dir, sprite, LAYER_TILES - 10)
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

GhostTreeRootMain::GhostTreeRootMain(const Vector& pos, GhostTreeAttack* parent):
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/main_root.sprite"),
  m_state(STATE_HATCHING),
  m_state_timer(),
  m_maxheight(),
  m_parent(parent),
  m_hill(Surface::from_file("images/creatures/mole/corrupted/root_base.png"))
{
  set_pos(get_pos() + Vector(-get_sprite()->get_current_hitbox_width() / 2.f, MAIN_ROOT_HATCH_OFFSET));
  set_start_position(get_pos());
  m_state_timer.start(MAIN_ROOT_HATCH_DURATION);

  SoundManager::current()->preload("sounds/brick.wav");
  SoundManager::current()->preload("sounds/darthit.wav");

  std::unique_ptr<SoundSource> sound = SoundManager::current()->create_sound_source("sounds/brick.wav");
  sound->set_position(get_pos());
  sound->set_pitch(MAIN_ROOT_SOUND_PITCH);
  sound->play();
  SoundManager::current()->manage_source(std::move(sound));

  const float gravity = Sector::get().get_gravity() * 100.f;
  for (int i = 0; i < 5; i++)
  {
    const Vector velocity(graphicsRandom.randf(-100.f, 100.f),
                          graphicsRandom.randf(-400.f, -300.f));
    Sector::get().add<SpriteParticle>("images/particles/corrupted_rock.sprite",
                                      "piece-" + std::to_string(i),
                                      get_pos() - Vector(0, MAIN_ROOT_HATCH_OFFSET + 10.f),
                                      ANCHOR_MIDDLE,
                                      velocity, Vector(0.f, gravity),
                                      LAYER_OBJECTS + 3, true);
  }
}

GhostTreeRootMain::~GhostTreeRootMain()
{
}

void
GhostTreeRootMain::active_update(float dt_sec)
{
  BadGuy::active_update(dt_sec);
  switch (m_state) {
    case STATE_HATCHING:
      if (m_state_timer.check())
      {
        m_state = STATE_RISING;
        m_state_timer.start(MAIN_ROOT_RISE_DURATION);
      }
      break;

    case STATE_RISING:
    {
      const double progress = static_cast<double>(m_state_timer.get_progress());
      const float target = get_height() + MAIN_ROOT_HATCH_OFFSET;
      const float off = QuadraticEaseIn(progress) * target;
      set_pos(get_pos().x, m_start_position.y - off);

      if (m_state_timer.check())
      {
        m_maxheight = get_pos().y;

        std::unique_ptr<SoundSource> sound = SoundManager::current()->create_sound_source("sounds/darthit.wav");
        sound->set_position(get_pos());
        sound->set_pitch(MAIN_ROOT_SOUND_PITCH);
        sound->play();
        SoundManager::current()->manage_source(std::move(sound));

        m_state = STATE_FALLING;
        m_state_timer.start(MAIN_ROOT_FALL_DURATION);
      }
      break;
    }

    case STATE_FALLING:
    {
      const double progress = static_cast<double>(m_state_timer.get_progress());
      const float target = get_height() + MAIN_ROOT_HATCH_OFFSET;
      const float off = QuadraticEaseIn(progress) * target;
      set_pos(get_pos().x, m_maxheight + off);

      if (m_state_timer.check())
      {
        m_parent->root_died();
        m_state = STATE_FADE_OUT;
        m_state_timer.start(0.2f);
      }
      break;
    }

    case STATE_FADE_OUT:
      if (m_state_timer.check())
        remove_me();

      break;

    default:
      break;
  }
}

void
GhostTreeRootMain::draw(DrawingContext& context)
{
  const Size orig = m_hill->get_region().get_size();
  const Sizef newsize = Sizef(orig.width, orig.height) * 1.6f;
  const Vector off((-newsize.width / 2.f) + (m_sprite->get_current_hitbox_width() / 2.f),
                   -newsize.height - MAIN_ROOT_HATCH_OFFSET - MAIN_ROOT_HILL_OFFSET);
  const Rectf dest(m_start_position + off, newsize);

  PaintStyle style;
  switch (m_state) {
    case STATE_HATCHING:
      style.set_alpha(std::min(1.f, m_state_timer.get_progress() * 2.f));
      break;

    case STATE_FADE_OUT:
      style.set_alpha(1.f - m_state_timer.get_progress());
      break;

    default:
      style.set_alpha(1.f);
      break;
  }

  context.color().draw_surface_scaled(m_hill, dest, m_layer + 5, style);

  BadGuy::draw(context);
}

GhostTreeRootRed::GhostTreeRootRed(const Vector& pos, GhostTreeAttack* parent):
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/red_root.sprite"),
  m_level_bottom(pos.y),
  m_level_top(pos.y),
  m_state(STATE_RISING),
  m_parent(parent)
{
  m_physic.set_velocity_y(-RED_ROOT_SPEED);

  const int variant = abs(static_cast<int>(pos.x)) % 3 + 1;
  set_action("variant" + std::to_string(variant));

  SoundManager::current()->preload("sounds/darthit.wav");

  m_level_top -= m_col.m_bbox.get_height();
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
        SoundManager::current()->play("sounds/darthit.wav", get_pos());
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

GhostTreeRootGreen::GhostTreeRootGreen(const Vector& pos, GhostTreeAttack* parent):
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/green_root.sprite"),
  m_level_top(pos.y),
  m_parent(parent)
{
  m_physic.set_velocity_y(-GREEN_ROOT_SPEED);
  m_level_top -= m_col.m_bbox.get_height();
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

  Sector::get().add<Explosion>(m_col.m_bbox.get_middle(), 0.f);

  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(100.f, -500.f),  GREEN_ROOT_SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(270.f, -350.f),  GREEN_ROOT_SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-100.f, -500.f), GREEN_ROOT_SHARD_SPRITE);
  Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-270.f, -350.f), GREEN_ROOT_SHARD_SPRITE);

  m_parent->root_died();
  remove_me();
}

GhostTreeRootBlue::GhostTreeRootBlue(const Vector& pos, GhostTreeAttack* parent):
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/granito_root.sprite"),
  m_level_bottom(pos.y),
  m_level_top(pos.y),
  m_state(STATE_RISING),
  m_state_timer(),
  m_variant(abs(static_cast<int>(pos.x)) % 2 + 1),
  m_parent(parent)
{
  m_physic.set_velocity_y(-BLUE_ROOT_SPEED);
  set_action("variant" + std::to_string(m_variant));
  m_level_top -= m_col.m_bbox.get_height();
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
        fire();
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

void
GhostTreeRootBlue::fire()
{
  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  if (m_variant == 1) {
    Sector::get().add<Dart>(get_pos() + Vector(-70.0f, 124.0f), Direction::LEFT, this, BLUE_ROOT_PROJECTILE,
                            "images/creatures/ghosttree/blue_root_light.sprite");
    Sector::get().add<Dart>(get_pos() + Vector( 16.0f,  57.0f), Direction::RIGHT, this, BLUE_ROOT_PROJECTILE,
                            "images/creatures/ghosttree/blue_root_light.sprite");
  } else {
    Sector::get().add<Dart>(get_pos() + Vector(-70.0f,  82.0f), Direction::LEFT, this, BLUE_ROOT_PROJECTILE,
                            "images/creatures/ghosttree/blue_root_light.sprite");
    Sector::get().add<Dart>(get_pos() + Vector( 16.0f, 124.0f), Direction::RIGHT, this, BLUE_ROOT_PROJECTILE,
                            "images/creatures/ghosttree/blue_root_light.sprite");
  }
}

GhostTreeRootPinch::GhostTreeRootPinch(const Vector& pos, GhostTreeAttack* parent):
  GhostTreeRoot(pos, Direction::AUTO, "images/creatures/ghosttree/pinch_root.sprite"),
  m_level_top(pos.y),
  m_state(STATE_RISING),
  m_state_timer(),
  m_parent(parent)
{
  m_physic.set_velocity_y(-PINCH_ROOT_SPEED);
  m_level_top -= m_col.m_bbox.get_height();
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
        fire();
        m_state = STATE_EXPLOSION_DELAY;
        m_state_timer.start(PINCH_ROOT_EXPLOSION_DELAY);
      }
      break;
    case STATE_EXPLOSION_DELAY:
      if (m_state_timer.check()) {
        Sector::get().add<Explosion>(m_col.m_bbox.get_middle(), EXPLOSION_STRENGTH_DEFAULT);

        Sector::get().add<Shard>(get_bbox().get_middle(), Vector(100.f, -500.f),  GREEN_ROOT_SHARD_SPRITE);
        Sector::get().add<Shard>(get_bbox().get_middle(), Vector(270.f, -350.f),  GREEN_ROOT_SHARD_SPRITE);
        Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-100.f, -500.f), GREEN_ROOT_SHARD_SPRITE);
        Sector::get().add<Shard>(get_bbox().get_middle(), Vector(-270.f, -350.f), GREEN_ROOT_SHARD_SPRITE);

        m_parent->root_died();
        remove_me();
      }
      break;
    default:
      break;
  }
}

void
GhostTreeRootPinch::fire()
{
  SoundManager::current()->play("sounds/dartfire.wav", get_pos());
  Sector::get().add<Dart>(get_pos() + Vector(-70.0f, 126.0f), Direction::LEFT, this, BLUE_ROOT_PROJECTILE,
                          "images/creatures/ghosttree/blue_root_light.sprite");
  Sector::get().add<Dart>(get_pos() + Vector( 16.0f, 127.0f), Direction::RIGHT, this, BLUE_ROOT_PROJECTILE,
                          "images/creatures/ghosttree/blue_root_light.sprite");
}

// PART 3: Root Attacks
// ----------------------------------------------------------------------------

GhostTreeAttackMain::GhostTreeAttackMain(Vector pos):
  m_spawn_timer(),
  m_pos(pos),
  m_remaining_roots(MAIN_ROOT_COUNT)
{
  m_spawn_timer.start(0.2f);
}

GhostTreeAttackMain::~GhostTreeAttackMain()
{
}

void
GhostTreeAttackMain::active_update(float dtime)
{
  using RaycastResult = CollisionSystem::RaycastResult;

  if (m_remaining_roots <= 0 || !m_spawn_timer.check())
    return;

  Player* p = Sector::get().get_nearest_player(m_pos);
  if (!p)
  {
    Sector::get().add<GhostTreeRootMain>(m_pos, this);
    return;
  }

  Vector pos(p->get_x() + (p->get_width() / 2), m_pos.y);

#if 0
  // Detect ground position below player
  // Disabled because the story mode arena works better without this
  // TODO: Make this configurable
  Vector eye(pos.x, p->get_bbox().get_bottom());
  RaycastResult result = Sector::get().get_first_line_intersection(eye, eye + Vector(0.f, 670.f),
                                                                   // CollisionSystem::IGNORE_OBJECTS, nullptr);

  if (result.is_valid)
    pos.y = result.box.get_top();
#endif

  Sector::get().add<GhostTreeRootMain>(pos, this);
  m_spawn_timer.start(MAIN_ROOT_DELAY);
}

bool
GhostTreeAttackMain::is_done() const
{
  return m_remaining_roots <= 0;
}

void
GhostTreeAttackMain::root_died()
{
  --m_remaining_roots;
}

GhostTreeAttackRed::GhostTreeAttackRed(float y, float x_start, float x_end):
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

  Sector::get().add<GhostTreeRootRed>(Vector(m_current_x, m_pos_y), this);
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

GhostTreeAttackGreen::GhostTreeAttackGreen(const Vector& pos):
  m_ended(false)
{
  Sector::get().add<GhostTreeRootGreen>(pos, this);
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

GhostTreeAttackBlue::GhostTreeAttackBlue(const Vector& pos):
  m_ended(false)
{
  Sector::get().add<GhostTreeRootBlue>(pos, this);
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

GhostTreeAttackPinch::GhostTreeAttackPinch(const Vector& pos, float x_left, float x_right):
  m_root_ended(false),
  m_left_trail(pos.y, pos.x - RED_ROOT_SPAN, x_left),
  m_right_trail(pos.y, pos.x + PINCH_ROOT_SPAN, x_right)
{
  Sector::get().add<GhostTreeRootPinch>(pos, this);
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

