//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#include "badguy/mrbomb.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "badguy/bomb.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

MrBomb::MrBomb(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right"),
  grabbed(false)
{
  walk_speed = 80;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Mr Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if( !reader.get( "sprite", sprite_name ) ){
    return;
  }
  if (sprite_name.empty()) {
    sprite_name = "images/creatures/mr_bomb/mr_bomb.sprite";
    return;
  }
  //Replace sprite
  sprite = SpriteManager::current()->create( sprite_name );
}

HitResponse
MrBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
MrBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if(grabbed)
    return FORCE_MOVE;
  return WalkingBadguy::collision_player(player, hit);
}

bool
MrBomb::collision_squished(GameObject& object)
{
  auto player = dynamic_cast<Player*>(&object);
  if(player && player->is_invincible()) {
    player->bounce(*this);
    kill_fall();
    return true;
  }
  if(is_valid()) {
    auto bomb = std::make_shared<Bomb>(get_pos(), dir, sprite_name);

    // Do not trigger dispenser because we need to wait for
    // the bomb instance to explode.
    if(get_parent_dispenser() != NULL)
    {
      bomb->set_parent_dispenser(get_parent_dispenser());
      set_parent_dispenser(NULL);
    }
    remove_me();
    Sector::current()->add_object(bomb);
  }
  kill_squished(object);
  return true;
}

void
MrBomb::active_update(float elapsed_time)
{
  if(grabbed)
    return;
  WalkingBadguy::active_update(elapsed_time);
}

void
MrBomb::kill_fall()
{
  if(is_valid()) {
    remove_me();
    auto explosion = std::make_shared<Explosion>(bbox.get_middle());
    Sector::current()->add_object(explosion);
  }

  run_dead_script();
}

void
MrBomb::ignite()
{
  kill_fall();
}

void
MrBomb::grab(MovingObject&, const Vector& pos, Direction dir_)
{
  assert(frozen);
  movement = pos - get_pos();
  dir = dir_;
  sprite->set_action(dir_ == LEFT ? "iced-left" : "iced-right");
  set_colgroup_active(COLGROUP_DISABLED);
  grabbed = true;
}

void
MrBomb::ungrab(MovingObject& , Direction dir_)
{
  dir = dir_;
  set_colgroup_active(COLGROUP_MOVING);
  grabbed = false;
}

bool
MrBomb::is_freezable() const
{
  return true;
}

bool
MrBomb::is_portable() const
{
  return frozen;
}

/* EOF */
