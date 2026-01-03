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

#include "object/tuxdoll.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"

static const float TUXDOLL_GRAVITY = .65f;
static const float TUXDOLL_VELOCITY_X = 100.f * TUXDOLL_GRAVITY;
static const float TUXDOLL_VELOCITY_Y = -400.f * TUXDOLL_GRAVITY;

TuxDoll::TuxDoll(const Vector& pos, Direction direction) :
  MovingSprite(pos, "images/powerups/1up/1up.sprite", LAYER_FLOATINGOBJECTS, COLGROUP_TOUCHABLE),
  physic()
{
  physic.set_velocity(TUXDOLL_VELOCITY_X * (direction == Direction::LEFT ? -1 : 1),
                      TUXDOLL_VELOCITY_Y);
  physic.set_gravity_modifier(TUXDOLL_GRAVITY);
}

void
TuxDoll::update(float dt_sec)
{
  if (!Sector::get().inside(m_col.m_bbox))
    remove_me();

  m_col.set_movement(physic.get_movement(dt_sec));
}

HitResponse
TuxDoll::collision(MovingObject& other, const CollisionHit& )
{
  auto player = dynamic_cast<Player*> (&other);
  if (player)
  {
    Sector::get().get_level().m_stats.increment_tuxdolls();
    SoundManager::current()->play("sounds/lifeup.wav", get_pos());

    remove_me();
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}
