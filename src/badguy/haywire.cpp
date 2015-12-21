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

#include "audio/sound_manager.hpp"
#include "badguy/bomb.hpp"
#include "badguy/haywire.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"
#include "util/reader.hpp"

#define TIME_EXPLOSION 5.0
#define TIME_STUNNED   0.5

Haywire::Haywire(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/haywire/haywire.sprite", "left", "right"),
  is_exploding(false),
  time_until_explosion(0.0f),
  is_stunned(false),
  time_stunned(0.0f),
  ticking(),
  grunting()
{
  walk_speed = 80;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Mr Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/haywire/haywire.sprite";
    return;
  }
  //Replace sprite
  sprite = SpriteManager::current()->create( sprite_name );
}

bool
Haywire::collision_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
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

  if(WalkingBadguy::is_frozen()) {
    WalkingBadguy::unfreeze();
  }

  if (!is_exploding) {
    set_action ((dir == LEFT) ? "ticking-left" : "ticking-right", /* loops = */ -1);
    walk_left_action = "ticking-left";
    walk_right_action = "ticking-right";
    set_walk_speed (160);
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

  time_stunned = TIME_STUNNED;
  is_stunned = true;
  physic.set_velocity_x (0.0);
  physic.set_acceleration_x (0.0);

  if (player)
    player->bounce (*this);

  return true;
}

void
Haywire::active_update(float elapsed_time)
{
  if (is_exploding) {
    ticking->set_position(get_pos());
    grunting->set_position(get_pos());
    if (elapsed_time >= time_until_explosion) {
      kill_fall ();
      return;
    }
    else
      time_until_explosion -= elapsed_time;
  }

  if (is_stunned) {
    if (time_stunned > elapsed_time) {
      time_stunned -= elapsed_time;
      return;
    }
    else { /* if (time_stunned <= elapsed_time) */
      elapsed_time -= time_stunned;
      time_stunned = 0.0;
      is_stunned = false;
    }
  }

  if (is_exploding) {
    Player *p = get_nearest_player ();
    float target_velocity = 0.0;

    if (p) {
      /* Player is on the right */
      if (p->get_pos ().x > this->get_pos ().x)
        target_velocity = walk_speed;
      else /* player in on the left */
        target_velocity = (-1.0) * walk_speed;
    } /* if (player) */

    WalkingBadguy::active_update(elapsed_time, target_velocity);
  }
  else {
    WalkingBadguy::active_update(elapsed_time);
  }
}

void
Haywire::kill_fall()
{
  if(is_exploding) {
    ticking->stop();
    grunting->stop();
  }
  if(is_valid()) {
    remove_me();
    auto explosion = std::make_shared<Explosion>(bbox.get_middle());
    Sector::current()->add_object(explosion);
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
    is_exploding = false;
    ticking->stop();
    grunting->stop();
  }
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
