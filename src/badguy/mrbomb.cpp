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
#include "object/portable.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/sector.hpp"
#include "util/reader_mapping.hpp"

MrBomb::MrBomb(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/mr_bomb/mr_bomb.sprite", "left", "right")
{
  walk_speed = 80;
  max_drop_height = 16;

  //Prevent stutter when Tux jumps on Mr Bomb
  SoundManager::current()->preload("sounds/explosion.wav");

  //Check if we need another sprite
  if ( !reader.get( "sprite", m_sprite_name ) ){
    return;
  }
  if (m_sprite_name.empty()) {
    m_sprite_name = "images/creatures/mr_bomb/mr_bomb.sprite";
    return;
  }
  //Replace sprite
  m_sprite = SpriteManager::current()->create( m_sprite_name );
}

HitResponse
MrBomb::collision(GameObject& object, const CollisionHit& hit)
{
  if (is_grabbed())
    return FORCE_MOVE;
  return WalkingBadguy::collision(object, hit);
}

HitResponse
MrBomb::collision_player(Player& player, const CollisionHit& hit)
{
  if (is_grabbed())
    return FORCE_MOVE;
  return WalkingBadguy::collision_player(player, hit);
}

bool
MrBomb::collision_squished(GameObject& object)
{
  if (m_frozen)
    return WalkingBadguy::collision_squished(object);

  auto player = dynamic_cast<Player*>(&object);
  if (player && player->is_invincible()) {
    player->bounce(*this);
    kill_fall();
    return true;
  }
  if (is_valid()) {
    auto& bomb = Sector::get().add<Bomb>(get_pos(), m_dir, m_sprite_name);

    // Do not trigger dispenser because we need to wait for
    // the bomb instance to explode.
    if (get_parent_dispenser() != nullptr)
    {
      bomb.set_parent_dispenser(get_parent_dispenser());
      set_parent_dispenser(nullptr);
    }

    remove_me();
  }
  kill_squished(object);
  return true;
}

void
MrBomb::active_update(float dt_sec)
{
  if (is_grabbed())
    return;
  WalkingBadguy::active_update(dt_sec);
}

void
MrBomb::kill_fall()
{
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

void
MrBomb::ignite()
{
  if (m_frozen)
    unfreeze();
  kill_fall();
}

void
MrBomb::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  assert(m_frozen);
  m_col.set_movement(pos - get_pos());
  m_dir = dir_;
  m_sprite->set_action(dir_ == Direction::LEFT ? "iced-left" : "iced-right");
  set_colgroup_active(COLGROUP_DISABLED);
}

bool
MrBomb::is_freezable() const
{
  return true;
}

bool
MrBomb::is_portable() const
{
  return m_frozen;
}

/* EOF */
