//  SuperTux - Hurting Platform
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include <config.h>
#include <stdexcept>

#include "hurting_platform.hpp"

#include "util/log.hpp"
#include "player.hpp"
#include "badguy/badguy.hpp"
#include "object_factory.hpp"

HurtingPlatform::HurtingPlatform(const lisp::Lisp& reader)
        : Platform(reader)
{
  set_group(COLGROUP_TOUCHABLE);
}

HitResponse
HurtingPlatform::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*>(&other);
  if (player) {
    if(player->is_invincible()) {
      return ABORT_MOVE;
    }
    player->kill(false);
  }
  BadGuy* badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy) {
    badguy->kill_fall();
  }

  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(HurtingPlatform, "hurting_platform");
