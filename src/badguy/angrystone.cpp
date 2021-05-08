//  AngryStone - A spiked block that charges towards the player
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

#include "badguy/angrystone.hpp"

#include "object/player.hpp"
#include "sprite/sprite.hpp"

static const float CHARGE_SPEED = 240;

static const float CHARGE_TIME = .5;
static const float ATTACK_TIME = 1;
static const float RECOVER_TIME = .5;

AngryStone::AngryStone(const ReaderMapping& reader) :
  BadGuy(reader, "images/creatures/angrystone/angrystone.sprite"),
  attackDirection(0.0f, 0.0f),
  oldWallDirection(0.0f, 0.0f),
  timer(),
  state(IDLE)
{
  m_countMe = false;
  m_physic.set_velocity_x(0);
  m_physic.set_velocity_y(0);
  m_physic.enable_gravity(true);
  m_sprite->set_action("idle");
}

void
AngryStone::collision_solid(const CollisionHit& /*hit*/)
{
  // TODO
#if 0
  if ((state == ATTACKING) &&
      (hit.normal.x == -attackDirection.x) && (hit.normal.y == attackDirection.y)) {
    state = IDLE;
    sprite->set_action("idle");
    physic.set_velocity_x(0);
    physic.set_velocity_y(0);
    physic.enable_gravity(true);
    oldWallDirection.x = attackDirection.x;
    oldWallDirection.y = attackDirection.y;
  }
#endif
}

void
AngryStone::kill_fall()
{
  //prevents AngryStone from getting killed by other enemies or the player
}

HitResponse
AngryStone::collision_badguy(BadGuy& badguy, const CollisionHit& )
{
  if (state == ATTACKING) {
    badguy.kill_fall();
    return FORCE_MOVE;
  }

  return FORCE_MOVE;
}

void
AngryStone::active_update(float dt_sec) {
  BadGuy::active_update(dt_sec);

  if (m_frozen) {
    return;
  }

  switch (state) {


    case IDLE: {
      auto player = get_nearest_player();
      if (player) {
        auto badguy = this;
        const Vector& playerPos = player->get_pos();
        const Vector& badguyPos = badguy->get_pos();
        float dx = (playerPos.x - badguyPos.x);
        float dy = (playerPos.y - badguyPos.y);

        float playerHeight = player->get_bbox().get_height();
        float badguyHeight = badguy->get_bbox().get_height();

        float playerWidth = player->get_bbox().get_width();
        float badguyWidth = badguy->get_bbox().get_width();

        if ((dx > -playerWidth) && (dx < badguyWidth)) {
          if (dy > 0) {
            attackDirection.x = 0;
            attackDirection.y = 1;
          } else {
            attackDirection.x = 0;
            attackDirection.y = -1;
          }
          if ((attackDirection.x != oldWallDirection.x) || (attackDirection.y != oldWallDirection.y)) {
            m_sprite->set_action("charging");
            timer.start(CHARGE_TIME);
            state = CHARGING;
          }
        } else if ((dy > -playerHeight) && (dy < badguyHeight)) {
          if (dx > 0) {
            attackDirection.x = 1;
            attackDirection.y = 0;
          } else {
            attackDirection.x = -1;
            attackDirection.y = 0;
          }
          if ((attackDirection.x != oldWallDirection.x) || (attackDirection.y != oldWallDirection.y)) {
            m_sprite->set_action("charging");
            timer.start(CHARGE_TIME);
            state = CHARGING;
          }
        }
      }
    } break;

    case CHARGING: {
      if (timer.check()) {
        m_sprite->set_action("attacking");
        timer.start(ATTACK_TIME);
        state = ATTACKING;
        m_physic.enable_gravity(false);
        m_physic.set_velocity_x(CHARGE_SPEED * attackDirection.x);
        m_physic.set_velocity_y(CHARGE_SPEED * attackDirection.y);
        oldWallDirection.x = 0;
        oldWallDirection.y = 0;
      }
    } break;

    case ATTACKING: {
      if (timer.check()) {
        timer.start(RECOVER_TIME);
        state = RECOVERING;
        m_sprite->set_action("idle");
        m_physic.enable_gravity(true);
        m_physic.set_velocity_x(0);
        m_physic.set_velocity_y(0);
      }
    } break;

    case RECOVERING: {
      if (timer.check()) {
        state = IDLE;
        m_sprite->set_action("idle");
        m_physic.enable_gravity(true);
        m_physic.set_velocity_x(0);
        m_physic.set_velocity_y(0);
      }
    } break;
  }

}

bool
AngryStone::is_freezable() const
{
  return state != ATTACKING;
}

bool
AngryStone::is_flammable() const
{
  return false;
}

/* EOF */
