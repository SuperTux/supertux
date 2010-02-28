//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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

#include "object/icecrusher.hpp"

#include "badguy/badguy.hpp"
#include "sprite/sprite.hpp"
#include "object/player.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

namespace {
/* Maximum movement speed in pixels per LOGICAL_FPS */
const float MAX_DROP_SPEED = 10.0;
const float RECOVER_SPEED = -3.125;
const float ACTIVATION_DISTANCE = 4.0;
const float PAUSE_TIME = 0.5;
}

IceCrusher::IceCrusher(const Reader& reader) :
  MovingSprite(reader, "images/creatures/icecrusher/icecrusher.sprite", LAYER_OBJECTS, COLGROUP_STATIC), 
  state(IDLE), 
  start_position(),
  physic(),
  cooldown_timer(0.0)
{
  start_position = get_bbox().p1;
  set_state(state, true);
}

/*
  IceCrusher::IceCrusher(const IceCrusher& other)
  : MovingSprite(other), 
  state(other.state), speed(other.speed) 
  {
  start_position = get_bbox().p1;
  set_state(state, true);
  }
*/
void 
IceCrusher::set_state(IceCrusherState state, bool force) 
{
  if ((this->state == state) && (!force)) return;
  switch(state) {
    case IDLE:
      set_group(COLGROUP_STATIC);
      physic.enable_gravity (false);
      sprite->set_action("idle");
      break;
    case CRUSHING:
      set_group(COLGROUP_MOVING_STATIC);
      physic.reset ();
      physic.enable_gravity (true);
      sprite->set_action("crushing");
      break;
    case RECOVERING:
      set_group(COLGROUP_MOVING_STATIC);
      physic.enable_gravity (false);
      sprite->set_action("recovering");
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
  this->state = state;
}

HitResponse
IceCrusher::collision(GameObject& other, const CollisionHit& hit)
{
  Player* player = dynamic_cast<Player*>(&other);

  /* If the other object is the player, and the collision is at the bottom of
   * the ice crusher, hurt the player. */
  if (player && hit.bottom) {
    if(player->is_invincible()) {
      if (state == CRUSHING)
        set_state(RECOVERING);
      return ABORT_MOVE;
    }
    player->kill(false);
    if (state == CRUSHING)
      set_state(RECOVERING);
    return FORCE_MOVE;
  }
  BadGuy* badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy) {
    badguy->kill_fall();
  }
  return FORCE_MOVE;
}
    
void 
IceCrusher::collision_solid(const CollisionHit& hit)
{
  switch(state) {
    case IDLE:
      break;
    case CRUSHING:
      if (hit.bottom) {
        cooldown_timer = PAUSE_TIME;
        set_state(RECOVERING);
      }
      break;
    case RECOVERING:
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

void
IceCrusher::update(float elapsed_time)
{
  if (cooldown_timer >= elapsed_time)
  {
    cooldown_timer -= elapsed_time;
    return;
  }
  else if (cooldown_timer != 0.0)
  {
    elapsed_time -= cooldown_timer;
    cooldown_timer = 0.0;
  }

  switch(state) {
    case IDLE:
      movement = Vector (0, 0);
      if (found_victim())
        set_state(CRUSHING);
      break;
    case CRUSHING:
      movement = physic.get_movement (elapsed_time);
      if (movement.y > MAX_DROP_SPEED)
        movement.y = MAX_DROP_SPEED;
      break;
    case RECOVERING:
      if (get_bbox().p1.y <= start_position.y+1) {
        set_pos(start_position);
        movement = Vector (0, 0);
        cooldown_timer = PAUSE_TIME;
        set_state(IDLE);
      }
      else {
        movement = Vector (0, RECOVER_SPEED);
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
}

bool
IceCrusher::found_victim()
{
  Player* player = Sector::current()->get_nearest_player (this->get_bbox ());
  if (!player) return false;

  const Rectf& player_bbox = player->get_bbox();
  const Rectf& crusher_bbox = get_bbox();
  if ((player_bbox.p1.y >= crusher_bbox.p2.y) /* player is below crusher */
      && (player_bbox.p2.x > (crusher_bbox.p1.x - ACTIVATION_DISTANCE))
      && (player_bbox.p1.x < (crusher_bbox.p2.x + ACTIVATION_DISTANCE)))
    return true;
  else
    return false;
}

/* EOF */
