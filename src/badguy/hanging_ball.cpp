//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "badguy/hanging_ball.hpp"

#include "audio/sound_manager.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
//#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

HangingBall::HangingBall(const ReaderMapping& reader) :
  Stalactite(reader)
{
  sprite = SpriteManager::current()->create("images/creatures/stalactite/hangingball.sprite");
  SoundManager::current()->preload("sounds/fire.ogg");
}

void
HangingBall::active_update(float elapsed_time)
{
  if(state == STALACTITE_HANGING && frozen) {
    return;
  }
  Stalactite::active_update(elapsed_time);
}

bool
HangingBall::is_freezable() const
{
  return state == STALACTITE_HANGING;
}

void
HangingBall::unfreeze() {
  BadGuy::unfreeze();
  sprite->set_action("normal");
}

HitResponse
HangingBall::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  if(state == STALACTITE_HANGING) {
    if(bullet.get_type() == FIRE_BONUS && !frozen) {
      fall();
      bullet.remove_me();
      SoundManager::current()->play("sounds/fire.ogg", get_pos());
      return FORCE_MOVE;
    } else {
      return BadGuy::collision_bullet(bullet, hit);
    }
  } else {
    return FORCE_MOVE;
  }
}

HitResponse
HangingBall::collision_player(Player& player, const CollisionHit& hit)
{
  if (frozen) {
    return BadGuy::collision_player(player, hit);
  } else {
    return Stalactite::collision_player(player, hit);
  }
}

HitResponse
HangingBall::collision_badguy(BadGuy& other, const CollisionHit& hit)
{
  if (frozen) {
    return BadGuy::collision_badguy(other, hit);
  } else {
    if (state == STALACTITE_SQUISHED) return FORCE_MOVE;

    // ignore other Stalactites
    if (dynamic_cast<Stalactite*>(&other)) return FORCE_MOVE;

    if (state != STALACTITE_FALLING) return BadGuy::collision_badguy(other, hit);
    other.kill_fall();

    return FORCE_MOVE;
  }
}

std::string
HangingBall::get_cracking_sound_path() const
{
  return "sounds/hanging_ball.wav";
}

/* EOF */
