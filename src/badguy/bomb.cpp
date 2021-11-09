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

#include "badguy/bomb.hpp"

#include "audio/sound_manager.hpp"
#include "audio/sound_source.hpp"
#include "object/explosion.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/sector.hpp"

Bomb::Bomb(const Vector& pos, Direction dir_, const std::string& custom_sprite /*= "images/creatures/mr_bomb/mr_bomb.sprite"*/ ) :
  BadGuy( pos, dir_, custom_sprite ),
  ticking(SoundManager::current()->create_sound_source("sounds/fizz.wav"))
{
  SoundManager::current()->preload("sounds/explosion.wav");
  set_action(dir_ == Direction::LEFT ? "ticking-left" : "ticking-right", 1);
  m_countMe = false;

  ticking->set_position(get_pos());
  ticking->set_looping(true);
  ticking->set_gain(1.0f);
  ticking->set_reference_distance(32);
  ticking->play();
}

void
Bomb::collision_solid(const CollisionHit& hit)
{
  if (is_grabbed()) {
    return;
  }
  if (hit.top || hit.bottom)
    m_physic.set_velocity_y(0);
  if (hit.left || hit.right)
    m_physic.set_velocity_x(-m_physic.get_velocity_x());
  if (hit.crush)
    m_physic.set_velocity(0, 0);

  update_on_ground_flag(hit);
}

HitResponse
Bomb::collision_player(Player& , const CollisionHit& )
{
  return ABORT_MOVE;
}

HitResponse
Bomb::collision_badguy(BadGuy& , const CollisionHit& )
{
  return ABORT_MOVE;
}

void
Bomb::active_update(float dt_sec)
{
  if (on_ground()) m_physic.set_velocity_x(0);

  ticking->set_position(get_pos());
  if (m_sprite->animation_done()) {
    explode();
  }
  else if (!is_grabbed()) {
    m_col.set_movement(m_physic.get_movement(dt_sec));
  }
}

void
Bomb::explode()
{
  ticking->stop();

  // Make the player let go before we explode, otherwise the player is holding
  // an invalid object. There's probably a better way to do this than in the
  // Bomb class.
  if (is_grabbed()) {
    auto player = dynamic_cast<Player*>(m_owner);

    if (player)
      player->stop_grabbing();
  }

  if (is_valid()) {
    remove_me();
    Sector::get().add<Explosion>(m_col.m_bbox.get_middle(),
      EXPLOSION_STRENGTH_DEFAULT);
  }

  run_dead_script();
}

void
Bomb::kill_fall()
{
  explode();
}

void
Bomb::ignite()
{
  explode();
}

void
Bomb::grab(MovingObject& object, const Vector& pos, Direction dir_)
{
  Portable::grab(object, pos, dir_);
  m_col.set_movement(pos - get_pos());
  m_dir = dir_;

  // We actually face the opposite direction of Tux here to make the fuse more
  // visible instead of hiding it behind Tux
  m_sprite->set_action_continued(m_dir == Direction::LEFT ? "ticking-right" : "ticking-left");
  set_colgroup_active(COLGROUP_DISABLED);
}

void
Bomb::ungrab(MovingObject& object, Direction dir_)
{
  m_dir = dir_;
  // This object is now thrown.
  int toss_velocity_x = 0;
  int toss_velocity_y = 0;
  auto player = dynamic_cast<Player*> (&object);

  // toss upwards
  if (dir_ == Direction::UP)
    toss_velocity_y += -500;

  // toss to the side when moving sideways
  if (player && player->get_physic().get_velocity_x()*(dir_ == Direction::LEFT ? -1 : 1) > 1) {
    toss_velocity_x += (dir_ == Direction::LEFT) ? -200 : 200;
    toss_velocity_y = (toss_velocity_y < -200) ? toss_velocity_y : -200;
    // toss farther when running
    if (player && player->get_physic().get_velocity_x()*(dir_ == Direction::LEFT ? -1 : 1) > 200)
      toss_velocity_x += static_cast<int>(player->get_physic().get_velocity_x() - (190.0f * (dir_ == Direction::LEFT ? -1.0f : 1.0f)));
  }

  m_physic.set_velocity(static_cast<float>(toss_velocity_x),
                      static_cast<float>(toss_velocity_y));

  set_colgroup_active(COLGROUP_MOVING);
  Portable::ungrab(object, dir_);
}

void Bomb::stop_looping_sounds()
{
  if (ticking) {
    ticking->stop();
  }
}

void Bomb::play_looping_sounds()
{
  if (ticking) {
    ticking->play();
  }
}

/* EOF */
