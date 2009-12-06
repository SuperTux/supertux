//  IceCrusher - A block to stand on, which can drop down to crush the player
//  Copyright (C) 2008 Christoph Sommer <christoph.sommer@2008.expires.deltadevelopment.de>
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
const float DROP_SPEED = 500;
const float RECOVER_SPEED = 200;
}

IceCrusher::IceCrusher(const Reader& reader) :
  MovingSprite(reader, "images/creatures/icecrusher/icecrusher.sprite", LAYER_OBJECTS, COLGROUP_STATIC), 
  state(IDLE), 
  start_position(),
  speed(Vector(0,0))
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
      speed=Vector(0,0);
      sprite->set_action("idle");
      break;
    case CRUSHING:
      set_group(COLGROUP_MOVING_STATIC);
      speed=Vector(0, DROP_SPEED);
      sprite->set_action("idle");
      break;
    case RECOVERING:
      set_group(COLGROUP_MOVING_STATIC);
      speed=Vector(0, -RECOVER_SPEED);
      sprite->set_action("idle");
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
  if (player && hit.bottom) {
    if(player->is_invincible()) {
      if (state == CRUSHING) set_state(RECOVERING);
      return ABORT_MOVE;
    }
    player->kill(false);
    if (state == CRUSHING) set_state(RECOVERING);
    return FORCE_MOVE;
  }
  BadGuy* badguy = dynamic_cast<BadGuy*>(&other);
  if (badguy) {
    badguy->kill_fall();
  }
  return FORCE_MOVE;
}
    
void 
IceCrusher::collision_solid(const CollisionHit& )
{
  switch(state) {
    case IDLE:
      break;
    case CRUSHING:
      set_state(RECOVERING);
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
  switch(state) {
    case IDLE:
      if (found_victim()) set_state(CRUSHING);
      break;
    case CRUSHING:
      break;
    case RECOVERING:
      if (get_bbox().p1.y <= start_position.y+1) {
        set_pos(start_position);
        set_state(IDLE);
      }
      break;
    default:
      log_debug << "IceCrusher in invalid state" << std::endl;
      break;
  }
  movement = speed * elapsed_time;
}

Player*
IceCrusher::get_nearest_player()
{
  // FIXME: does not really return nearest player

  std::vector<Player*> players = Sector::current()->get_players();
  for (std::vector<Player*>::iterator playerIter = players.begin(); playerIter != players.end(); ++playerIter) {
    Player* player = *playerIter;
    if (player->is_dying() || player->is_dead()) continue;
    return player;
  }

  return 0;
}

bool
IceCrusher::found_victim()
{
  Player* player = this->get_nearest_player();
  if (!player) return false;

  const Rectf& pr = player->get_bbox();
  const Rectf& br = get_bbox();
  if ((pr.p2.x > br.p1.x) && (pr.p1.x < br.p2.x) && (pr.p1.y >= br.p2.y)) {
    return true;
  }
  return false;
}

/* EOF */
