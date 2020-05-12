//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "badguy/haywire.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

namespace {

const float TIME_EXPLOSION = 5.0f;
const float TIME_STUNNED = 0.5f;

const float NORMAL_WALK_SPEED = 80.0f;
const float EXPLODING_WALK_SPEED = 160.0f;

} // namespace

Haywire::Haywire(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/haywire/haywire.sprite", "left", "right"),
  is_exploding(false),
  time_until_explosion(0.0f),
  is_stunned(false),
  time_stunned(0.0f),
  ticking(),
  grunting()
{
  walk_speed = NORMAL_WALK_SPEED;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Mr Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if ( !reader.get( "sprite", m_sprite_name ) ){
    return;
  }
  if (m_sprite_name.empty()) {
    m_sprite_name = "images/creatures/haywire/haywire.sprite";
    return;
  }
  //Replace sprite
  m_sprite = SpriteManager::current()->create( m_sprite_name );
}

bool
Haywire::collision_squished(GameObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if (player && player->is_invincible()) {
    player->bounce (*this);
    kill_fall();
    return true;
  }

  if (is_stunned) {
    if (player)
      player->bounce (*this);
    return true;
  }

  if (WalkingBadguy::is_frozen()) {
    WalkingBadguy::unfreeze();
  }

  if (!is_exploding) {
    start_exploding();
  }

  time_stunned = TIME_STUNNED;
  is_stunned = true;
  m_physic.set_velocity_x(0.f);
  m_physic.set_acceleration_x(0.f);

  if (player)
    player->bounce (*this);

  return true;
}

void
Haywire::active_update(float dt_sec)
{
  if (is_exploding) {
    ticking->set_position(get_pos());
    grunting->set_position(get_pos());
    if (dt_sec >= time_until_explosion) {
      kill_fall ();
      return;
    }
    else
      time_until_explosion -= dt_sec;
  }

  if (is_stunned) {
    if (time_stunned > dt_sec) {
      time_stunned -= dt_sec;
    }
    else { /* if (time_stunned <= dt_sec) */
      time_stunned = 0.f;
      is_stunned = false;
    }
  }

  if (is_exploding) {
    auto p = get_nearest_player ();
    float target_velocity = 0.f;

    if (p && time_stunned == 0.f) {
      /* Player is on the right */
      if (p->get_pos ().x > get_pos ().x)
        target_velocity = walk_speed;
      else /* player in on the left */
        target_velocity = (-1.f) * walk_speed;
    }

    WalkingBadguy::active_update(dt_sec, target_velocity);
  }
  else {
    WalkingBadguy::active_update(dt_sec);
  }
}

void
Haywire::kill_fall()
{
  if (is_exploding) {
    ticking->stop();
    grunting->stop();
  }
  if (is_valid()) {
    remove_me();
    Sector::get().add<Explosion>(m_col.m_bbox.get_middle(),
      EXPLOSION_STRENGTH_DEFAULT);
  }

  run_dead_script();
}

bool
Haywire::is_freezable() const
{
  return true;
}

void
Haywire::ignite()
{
  kill_fall();
}

void
Haywire::freeze() {
  BadGuy::freeze();
  if (is_exploding) {
    stop_exploding();
  }
}

void
Haywire::start_exploding()
{
  set_action ((m_dir == Direction::LEFT) ? "ticking-left" : "ticking-right", /* loops = */ -1);
  walk_left_action = "ticking-left";
  walk_right_action = "ticking-right";
  set_walk_speed (EXPLODING_WALK_SPEED);
  time_until_explosion = TIME_EXPLOSION;
  is_exploding = true;

  ticking = SoundManager::current()->create_sound_source("sounds/fizz.wav");
  ticking->set_position(get_pos());
  ticking->set_looping(true);
  ticking->set_reference_distance(32);
  ticking->play();
  grunting = SoundManager::current()->create_sound_source("sounds/grunts.ogg");
  grunting->set_position(get_pos());
  grunting->set_looping(true);
  grunting->set_reference_distance(32);
  grunting->play();
}

void
Haywire::stop_exploding()
{
  walk_left_action = "left";
  walk_right_action = "right";
  set_walk_speed(NORMAL_WALK_SPEED);
  time_until_explosion = 0.0f;
  is_exploding = false;

  if (ticking)
    ticking->stop();

  if (grunting)
    grunting->stop();
}

void Haywire::stop_looping_sounds()
{
  if (ticking) {
    ticking->stop();
  }
  if (grunting) {
    grunting->stop();
  }
}

void Haywire::play_looping_sounds()
{
  if (is_exploding) {
    if (ticking) {
      ticking->play();
    }
    if (grunting) {
      grunting->play();
    }
  }
}

/* EOF */
