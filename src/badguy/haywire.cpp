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
const float STOMPED_TIME = 1.0f;
const float TIME_STUNNED = 0.5f;

const float NORMAL_WALK_SPEED = 80.0f;
const float EXPLODING_WALK_SPEED = 200.0f;

} // namespace

Haywire::Haywire(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/haywire/haywire.sprite", "left", "right"),
  is_exploding(false),
  time_until_explosion(0.0f),
  is_stunned(false),
  time_stunned(0.0f),
  ticking(),
  grunting(),
  stomped_timer()
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
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

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
	stomped_timer.start(STOMPED_TIME);
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
  auto* player = get_nearest_player();

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

  if (is_exploding)
  {
    if (on_ground() && std::abs(m_physic.get_velocity_x()) > 40.f && player)
    {
      //jump over 1-tall roadblocks
      Rectf jump_box = get_bbox();
      jump_box.set_left(m_col.m_bbox.get_left() + (m_dir == Direction::LEFT ? -48.f : 38.f));
      jump_box.set_right(m_col.m_bbox.get_right() + (m_dir == Direction::RIGHT ? 48.f : -38.f));

      Rectf exception_box = get_bbox();
      exception_box.set_left(m_col.m_bbox.get_left() + (m_dir == Direction::LEFT ? -48.f : 38.f));
      exception_box.set_right(m_col.m_bbox.get_right() + (m_dir == Direction::RIGHT ? 48.f : -38.f));
      exception_box.set_top(m_col.m_bbox.get_top() - 32.f);
      exception_box.set_bottom(m_col.m_bbox.get_bottom() - 48.f);

      if (!Sector::get().is_free_of_statics(jump_box) && Sector::get().is_free_of_statics(exception_box))
      {
        m_physic.set_velocity_y(-325.f);
      }
      else
      {
        //jump over gaps if Tux isnt below
        Rectf gap_box = get_bbox();
        gap_box.set_left(m_col.m_bbox.get_left() + (m_dir == Direction::LEFT ? -38.f : 26.f));
        gap_box.set_right(m_col.m_bbox.get_right() + (m_dir == Direction::LEFT ? -26.f : 38.f));
        gap_box.set_top(m_col.m_bbox.get_top());
        gap_box.set_bottom(m_col.m_bbox.get_bottom() + 28.f);

        if (Sector::get().is_free_of_statics(gap_box)
          && (player->get_bbox().get_bottom() <= m_col.m_bbox.get_bottom()))
        {
          m_physic.set_velocity_y(-325.f);
        }
      }
    }

    //end of pathfinding

	  if (stomped_timer.get_timeleft() < 0.05f) {
        set_action ((m_dir == Direction::LEFT) ? "ticking-left" : "ticking-right", /* loops = */ -1);
        walk_left_action = "ticking-left";
        walk_right_action = "ticking-right";
    }
    else {
        set_action ((m_dir == Direction::LEFT) ? "active-left" : "active-right", /* loops = */ 1);
        walk_left_action = "active-left";
	      walk_right_action = "active-right";
    }

    float target_velocity = 0.f;

    if (!m_frozen)
    {
      if (stomped_timer.get_timeleft() >= 0.05f)
      {
        target_velocity = 0.f;
      }
      else if (player && time_stunned == 0.0f)
      {
        /* Player is on the right or left*/
        target_velocity = (player->get_pos().x > get_pos().x) ? walk_speed : (-1.f) * walk_speed;
      }
    }
    else
    {
      target_velocity = 0.f;
    }
    WalkingBadguy::active_update(dt_sec, target_velocity, 3.f);
  }
  else
  {
    WalkingBadguy::active_update(dt_sec);
  }
}

void
Haywire::deactivate()
{
  // stop ticking/grunting sounds, in case we are deactivated before actually
  // exploding (see https://github.com/SuperTux/supertux/issues/1260)
  stop_looping_sounds();
}

void
Haywire::kill_fall()
{
  if (is_exploding) {
    ticking->stop();
    grunting->stop();
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
  }
}

bool
Haywire::is_freezable() const
{
  return true;
}

void
Haywire::ignite()
{
  if (m_frozen)
    unfreeze();
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
  set_walk_speed (EXPLODING_WALK_SPEED);
  max_drop_height = -1;
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
  max_drop_height = 16;
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

HitResponse Haywire::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  if (is_exploding)
  {
    badguy.kill_fall();
    return FORCE_MOVE;
  }
  if (m_frozen)
    return FORCE_MOVE;
  else
  {
    WalkingBadguy::collision_badguy(badguy, hit);
  }
  return ABORT_MOVE;
}

/* EOF */
