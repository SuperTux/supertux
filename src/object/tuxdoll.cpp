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

TuxDoll::TuxDoll(const Vector& pos) :
  MovingSprite(pos, "images/powerups/1up/1up.sprite", LAYER_OBJECTS + 10, COLGROUP_TOUCHABLE)
{
  SoundManager::current()->preload("sounds/lifeup.wav");
}

void
TuxDoll::update(float dt_sec)
{
  m_sprite->set_angle(m_sprite->get_angle() + 5 * dt_sec);
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
