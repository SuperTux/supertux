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
  is_stunned(false)
{
  walk_speed = 80;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Mr Bomb
  sound_manager->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if( sprite_name == "" ){
    sprite_name = "images/creatures/haywire/haywire.sprite";
    return;
  }
  //Replace sprite
  sprite = sprite_manager->create( sprite_name );
}

/* Haywire created by a dispenser always gets default sprite atm.*/
Haywire::Haywire(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/haywire/haywire.sprite", "left", "right"),
  is_exploding(false),
  is_stunned(false)
{
  walk_speed = 80;
  max_drop_height = 16;
  sound_manager->preload("sounds/explosion.wav");
}

HitResponse
Haywire::collision(GameObject& object, const CollisionHit& hit)
{
  return WalkingBadguy::collision(object, hit);
}

HitResponse
Haywire::collision_player(Player& player, const CollisionHit& hit)
{
  return WalkingBadguy::collision_player(player, hit);
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
    player->bounce (*this);
    return true;
  }

  if (!is_exploding) {
    set_action ((dir == LEFT) ? "ticking-left" : "ticking-right", /* loops = */ -1);
    walk_left_action = "ticking-left";
    walk_right_action = "ticking-right";
    set_walk_speed (160);
    time_until_explosion = TIME_EXPLOSION;
    is_exploding = true;
  }

  time_stunned = TIME_STUNNED;
  is_stunned = true;

  player->bounce (*this);
  return true;
}

void
Haywire::active_update(float elapsed_time)
{
  if (is_exploding) {
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

  if (is_exploding && !turn_around_timer.started()) {
    Player *p = this->get_nearest_player ();

    if (p) {
      Direction player_dir = LEFT;

      if (p->get_pos ().x > this->get_pos ().x)
        player_dir = RIGHT;

      if (player_dir != dir)
        turn_around ();
    }
  }

  WalkingBadguy::active_update(elapsed_time);
}

void
Haywire::kill_fall()
{
  if(is_valid()) {
    remove_me();
    Explosion* explosion = new Explosion(get_bbox().get_middle());
    Sector::current()->add_object(explosion);
  }

  run_dead_script();
}

void
Haywire::freeze()
{
  WalkingBadguy::freeze();
  sprite->set_action(dir == LEFT ? "iced-left" : "iced-right");
}

bool
Haywire::is_freezable() const
{
  return true;
}

/* vim: set sw=2 sts=2 et : */
/* EOF */
