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
#include "badguy/owl.hpp"
#include "object/coin_explode.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "object/portable.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

GoldBomb::GoldBomb(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/gold_bomb/gold_bomb.sprite", "left", "right"),
  tstate(STATE_NORMAL),
  ticking(),
  m_exploding_sprite(SpriteManager::current()->create("images/creatures/mr_bomb/ticking_glow/ticking_glow.sprite"))
{
  walk_speed = 80;
  max_drop_height = 16;

  SoundManager::current()->preload("sounds/explosion.wav");

  m_exploding_sprite->set_action("default", 1);
}

void
GoldBomb::collision_solid(const CollisionHit& hit)
{
  if (tstate == STATE_TICKING) {
    if (hit.bottom) {
      m_physic.set_velocity_y(0);
      m_physic.set_velocity_x(0);
    }else if (hit.left || hit.right)
      m_physic.set_velocity_x(-m_physic.get_velocity_x());
    else if (hit.top)
      m_physic.set_velocity_y(0);
    update_on_ground_flag(hit);
    return;
  }
  WalkingBadguy::collision_solid(hit);
}

HitResponse
GoldBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING) {
    if ( dynamic_cast<Player*>(&object) ) {
      return ABORT_MOVE;
    }
    if ( dynamic_cast<BadGuy*>(&object)) {
      return ABORT_MOVE;
    }
  }
  if (is_grabbed())
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
GoldBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING)
    return FORCE_MOVE;
  if (is_grabbed())
    return FORCE_MOVE;
  return BadGuy::collision_player(player, hit);
}

HitResponse
GoldBomb::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (tstate == STATE_TICKING)
    return FORCE_MOVE;
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
  if (is_valid() && tstate == STATE_NORMAL) {
    tstate = STATE_TICKING;
    m_frozen = false;
    set_action(m_dir == Direction::LEFT ? "ticking-left" : "ticking-right", 1);
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
  if (is_grabbed())
    return;
  WalkingBadguy::active_update(dt_sec);
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
      Sector::get().add<CoinExplode>(get_pos() + Vector(0, -40), !m_parent_dispenser);
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
  if (tstate == STATE_NORMAL) {
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

/* EOF */
