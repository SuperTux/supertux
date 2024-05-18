//  SuperTux BadGuy GoldBomb - a bomb that throws up coins when exploding
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2013 LMH <lmh.0013@gmail.com>
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

#include "badguy/goldbomb.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "badguy/bomb.hpp"
#include "badguy/haywire.hpp"
#include "badguy/owl.hpp"
#include "object/coin_explode.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

static const float HOP_HEIGHT = -250.f;
static const float REALIZE_TIME = 0.5f;

// SAFE_DIST >= REALIZE_DIST
static const float REALIZE_DIST = 32.f * 8.f;
static const float SAFE_DIST = 32.f * 10.f;

static const float NORMAL_WALK_SPEED = 80.0f;
static const float FLEEING_WALK_SPEED = 180.0f;

GoldBomb::GoldBomb(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/gold_bomb/gold_bomb.sprite", "left", "right"),
  tstate(STATE_NORMAL),
  m_realize_timer(),
  ticking(),
  m_exploding_sprite(SpriteManager::current()->create("images/creatures/mr_bomb/ticking_glow/ticking_glow.sprite"))
{
  assert(SAFE_DIST >= REALIZE_DIST);

  walk_speed = NORMAL_WALK_SPEED;
  set_ledge_behavior(LedgeBehavior::SMART);

  SoundManager::current()->preload("sounds/explosion.wav");

  m_exploding_sprite->set_action("default", 1);
}

void
GoldBomb::collision_solid(const CollisionHit& hit)
{
  if (tstate == STATE_TICKING) {
    if (hit.bottom)
      m_physic.set_velocity(0, 0);
    else
      kill_fall();

    update_on_ground_flag(hit);
    return;
  } else if (tstate != STATE_NORMAL && (hit.left || hit.right)) {
    cornered();
    return;
  }

  WalkingBadguy::collision_solid(hit);
}

HitResponse
GoldBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING)
  {
    auto player = dynamic_cast<Player*>(&object);
    if (player) return collision_player(*player, hit);
    auto badguy = dynamic_cast<BadGuy*>(&object);
    if (badguy) return collision_badguy(*badguy, hit);
  }

  if (is_grabbed())
    return FORCE_MOVE;

  return WalkingBadguy::collision(object, hit);
}

HitResponse
GoldBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING)
  {
    if (m_physic.get_velocity() != Vector())
      kill_fall();
    return ABORT_MOVE;
  }
  if (is_grabbed())
    return FORCE_MOVE;
  return BadGuy::collision_player(player, hit);
}

HitResponse
GoldBomb::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING)
  {
    if (m_physic.get_velocity() != Vector()) kill_fall();
    return ABORT_MOVE;
  } else if (tstate != STATE_NORMAL) {
    return FORCE_MOVE;
  }
  return WalkingBadguy::collision_badguy(badguy, hit);
}

bool
GoldBomb::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  Player* player = dynamic_cast<Player*>(&object);
  if (player && player->is_invincible()) {
    player->bounce(*this);
    kill_fall();
    return true;
  }
  if (is_valid() && tstate != STATE_TICKING) {
    tstate = STATE_TICKING;
    m_frozen = false;
    set_action("ticking", m_dir, 1);
    m_physic.set_velocity_x(0);

    if (player)
      player->bounce(*this);
    SoundManager::current()->play("sounds/squish.wav", get_pos());
    ticking = SoundManager::current()->create_sound_source("sounds/fizz.wav");
    ticking->set_position(get_pos());
    ticking->set_looping(true);
    ticking->set_gain(1.0f);
    ticking->set_reference_distance(32);
    ticking->play();
  }
  return true;
}

void
GoldBomb::active_update(float dt_sec)
{
  if (tstate == STATE_TICKING) {
    m_exploding_sprite->set_action("exploding", 1);
    if (on_ground()) m_physic.set_velocity_x(0);
    ticking->set_position(get_pos());
    if (m_sprite->animation_done()) {
      kill_fall();
    }
    else if (!is_grabbed()) {
      m_col.set_movement(m_physic.get_movement(dt_sec));
    }
    return;
  }

  if ((tstate == STATE_FLEEING || tstate == STATE_CORNERED) && on_ground() && might_fall(s_normal_max_drop_height+1))
  {
    // also check for STATE_CORNERED just so
    // the bomb doesnt automatically turn around
    cornered();
    return;
  }
  WalkingBadguy::active_update(dt_sec);

  if (m_frozen) return;

  // Look for any of these in safe distance:
  // Player, ticking Haywire, ticking Bomb or ticking GoldBomb
  MovingObject* obj = nullptr;
  std::vector<MovingObject*> objs = Sector::get().get_nearby_objects(get_bbox().get_middle(), SAFE_DIST);
  for (MovingObject* currobj : objs)
  {
    obj = currobj;

    auto player = dynamic_cast<Player*>(obj);
    if (player && !player->get_ghost_mode()) break;

    auto haywire = dynamic_cast<Haywire*>(obj);
    if (haywire && haywire->is_exploding()) break;

    auto bomb = dynamic_cast<Bomb*>(obj);
    if (bomb) break;

    auto goldbomb = dynamic_cast<GoldBomb*>(obj);
    if (goldbomb && goldbomb->is_ticking()) break;

    obj = nullptr;
  }

  if (!obj)
  {
    // Everybody's outside of safe distance. Am I cornered?

    if (tstate == STATE_CORNERED)
    {
      // Look back to check.
      set_action("recover", m_dir);
      if (!m_sprite->animation_done()) return;
    }

    // Finally, when done recovering, go back to normal.
    if (tstate == STATE_NORMAL) return;

    tstate = STATE_NORMAL;
    m_physic.set_velocity_x(NORMAL_WALK_SPEED * (m_dir == Direction::LEFT ? -1 : 1));
    m_physic.set_acceleration_x(0);
    set_action(m_dir);
    set_ledge_behavior(LedgeBehavior::SMART);
    set_walk_speed(NORMAL_WALK_SPEED);
    return;
  }

  // Someone's in safe distance
  const Vector p1      = get_bbox().get_middle();
  const Vector p2      = obj->get_bbox().get_middle();
  const Vector vecdist = p2-p1;

  // But I only react to those who are in realize distance
  if (glm::length(vecdist) > REALIZE_DIST && tstate == STATE_NORMAL) return;

  // Someone's around!
  switch (tstate)
  {
    case STATE_FLEEING:
      // They popped up from the other side! Turn around!
      if (m_dir == (vecdist.x > 0 ? Direction::LEFT : Direction::RIGHT)) return;
      [[fallthrough]];

    case STATE_NORMAL:
    {
      if (!on_ground()) break;

      // Gold bomb is solid therefore raycast from
      // one of the upper corners of the hitbox.
      // (grown 1 just to make sure it doesnt interfere.)
      const Rectf eye = get_bbox().grown(1.f);
      if (!Sector::get().free_line_of_sight(
        vecdist.x <= 0 ? eye.p1() : Vector(eye.get_right(), eye.get_top()),
        obj->get_bbox().get_middle(),
        false,
        obj
      )) break;

      // Hop before fleeing.
      set_walk_speed(0);
      m_physic.set_velocity_y(HOP_HEIGHT);
      m_physic.set_velocity_x(0);
      m_physic.set_acceleration_x(0);
      m_dir = vecdist.x > 0 ? Direction::RIGHT : Direction::LEFT;
      m_sprite->set_action("flee", m_dir);
      tstate = STATE_REALIZING;
      m_realize_timer.start(REALIZE_TIME);
      break;
    }

    case STATE_REALIZING:
      if (!m_realize_timer.check()) break;

      flee(vecdist.x > 0 ? Direction::LEFT : Direction::RIGHT);
      break;

    default: break;
  }
}

void
GoldBomb::draw(DrawingContext& context)
{
  m_sprite->draw(context.color(), get_pos(), m_layer, m_flip);

  if (tstate == STATE_TICKING)
  {
    m_exploding_sprite->set_blend(Blend::ADD);
    m_exploding_sprite->draw(context.light(),
      get_pos() + Vector(get_bbox().get_width() / 2, get_bbox().get_height() / 2), m_layer, m_flip);
  }
  WalkingBadguy::draw(context);
}

void
GoldBomb::kill_fall()
{
  if (tstate == STATE_TICKING)
    ticking->stop();

  // Make the player let go before we explode, otherwise the player is holding
  // an invalid object. There's probably a better way to do this than in the
  // GoldBomb class.
  if (is_grabbed()) {
    Player* player = dynamic_cast<Player*>(m_owner);

    if (player)
      player->stop_grabbing();
  }

  if (is_valid()) {
    if (m_frozen)
      BadGuy::kill_fall();
    else
    {
      remove_me();
      Sector::get().add<Explosion>(m_col.m_bbox.get_middle(),
        EXPLOSION_STRENGTH_DEFAULT);
      run_dead_script();
    }
      Sector::get().add<CoinExplode>(get_pos(), !m_parent_dispenser);
  }
}

void
GoldBomb::ignite()
{
  if (m_frozen)
    unfreeze();
  kill_fall();
}

void
GoldBomb::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object,pos,dir_);
  if (tstate == STATE_TICKING){
    // We actually face the opposite direction of Tux here to make the fuse more
    // visible instead of hiding it behind Tux.
    set_action("ticking", m_dir, Sprite::LOOPS_CONTINUED);
    set_colgroup_active(COLGROUP_DISABLED);
  }
  else if (m_frozen){
    set_action("iced", dir_);
  }
  else if (dynamic_cast<Owl*>(&object))
    set_action(dir_);
  m_col.set_movement(pos - get_pos());
  m_dir = dir_;
  set_colgroup_active(COLGROUP_DISABLED);
}

void
GoldBomb::ungrab(MovingObject& object, Direction dir_)
{
  auto player = dynamic_cast<Player*> (&object);
  if (m_frozen)
    BadGuy::ungrab(object, dir_);
  else
  {
    // Handle swimming state of the player.
    if (player && (player->is_swimming() || player->is_water_jumping()))
    {
      float swimangle = player->get_swimming_angle();
      m_physic.set_velocity(Vector(std::cos(swimangle) * 40.f, std::sin(swimangle) * 40.f) +
        player->get_physic().get_velocity());
    }
    // Handle non-swimming.
    else
    {
      if (player)
      {
        // Handle x-movement based on the player's direction and velocity.
        if (fabsf(player->get_physic().get_velocity_x()) < 1.0f)
          m_physic.set_velocity_x(0.f);
        else if ((player->m_dir == Direction::LEFT && player->get_physic().get_velocity_x() <= -1.0f)
          || (player->m_dir == Direction::RIGHT && player->get_physic().get_velocity_x() >= 1.0f))
          m_physic.set_velocity_x(player->get_physic().get_velocity_x()
            + (player->m_dir == Direction::LEFT ? -10.f : 10.f));
        else
          m_physic.set_velocity_x(player->get_physic().get_velocity_x()
            + (player->m_dir == Direction::LEFT ? -330.f : 330.f));
        // Handle y-movement based on the player's direction and velocity.
        m_physic.set_velocity_y(dir_ == Direction::UP ? -500.f :
          dir_ == Direction::DOWN ? 500.f :
          player->get_physic().get_velocity_x() != 0.f ? -200.f : 0.f);
      }
    }
  }
  set_colgroup_active(m_frozen ? COLGROUP_MOVING_STATIC : COLGROUP_MOVING);
  Portable::ungrab(object, dir_);
}

void
GoldBomb::freeze()
{
  if (tstate != STATE_TICKING) {
    tstate = STATE_NORMAL;
    WalkingBadguy::freeze();
  }
}

bool
GoldBomb::is_freezable() const
{
  return true;
}

bool
GoldBomb::is_portable() const
{
  return (m_frozen || (tstate == STATE_TICKING));
}

void GoldBomb::stop_looping_sounds()
{
  if (ticking) {
    ticking->stop();
  }
}

void GoldBomb::play_looping_sounds()
{
  if (tstate == STATE_TICKING && ticking) {
    ticking->play();
  }
}

void
GoldBomb::flee(Direction dir)
{
  set_walk_speed(FLEEING_WALK_SPEED);
  set_ledge_behavior(LedgeBehavior::NORMAL);
  m_dir = dir;

  const float speed = FLEEING_WALK_SPEED * (m_dir == Direction::LEFT ? -1 : 1);
  m_physic.set_acceleration_x(speed);
  m_physic.set_velocity_x(speed);

  if (get_action() == dir_to_string(m_dir))
    m_sprite->set_animation_loops(-1);
  else
    set_action("flee", m_dir);

  tstate = STATE_FLEEING;
}

void
GoldBomb::cornered()
{
  if (tstate == STATE_CORNERED) return;

  set_walk_speed(0);
  m_physic.set_velocity_x(0);
  m_physic.set_acceleration_x(0);

  set_action("scared", m_dir);

  tstate = STATE_CORNERED;
}

/* EOF */
