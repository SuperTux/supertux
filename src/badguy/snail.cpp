//  SuperTux - Badguy "Snail"
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

#include "badguy/snail.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

#include <math.h>

namespace {
const float SNAIL_KICK_SPEED = 500;
const int MAX_SNAIL_SQUISHES = 10;
const float SNAIL_KICK_SPEED_Y = -500; /**< y-velocity gained when kicked */
}

Snail::Snail(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/snail/snail.sprite", "left", "right"), 
  state(STATE_NORMAL), 
  kicked_delay_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
}

Snail::Snail(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/snail/snail.sprite", "left", "right"), 
  state(STATE_NORMAL), 
  kicked_delay_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
}

void
Snail::initialize()
{
  WalkingBadguy::initialize();
  be_normal();
}

void
Snail::be_normal()
{
  if (state == STATE_NORMAL) return;

  state = STATE_NORMAL;
  WalkingBadguy::initialize();
}

void
Snail::be_flat()
{
  state = STATE_FLAT;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right", 1);

  physic.set_velocity_x(0);
  physic.set_velocity_y(0);
}

void
Snail::be_kicked()
{
  state = STATE_KICKED_DELAY;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right", 1);

  physic.set_velocity_x(0);
  physic.set_velocity_y(0);

  // start a timer to delay addition of upward movement until we are (hopefully) out from under the player
  kicked_delay_timer.start(0.05f);
}

bool
Snail::can_break(){
  return state == STATE_KICKED;
}

void
Snail::active_update(float elapsed_time)
{
  switch (state) {

    case STATE_NORMAL:
      WalkingBadguy::active_update(elapsed_time);
      return;

    case STATE_FLAT:
      if (sprite->animation_done()) {
        be_normal();
      }
      break;

    case STATE_KICKED_DELAY:
      if (kicked_delay_timer.check()) {
        physic.set_velocity_x(dir == LEFT ? -SNAIL_KICK_SPEED : SNAIL_KICK_SPEED);
        physic.set_velocity_y(SNAIL_KICK_SPEED_Y);
        state = STATE_KICKED;
      }
      break;

    case STATE_KICKED:
      physic.set_velocity_x(physic.get_velocity_x() * pow(0.99, elapsed_time/0.02));
      if (sprite->animation_done() || (fabsf(physic.get_velocity_x()) < walk_speed)) be_normal();
      break;

  }

  BadGuy::active_update(elapsed_time);
}

void
Snail::collision_solid(const CollisionHit& hit)
{
  switch (state) {
    case STATE_NORMAL:
      WalkingBadguy::collision_solid(hit);
      return;
    case STATE_KICKED:
      if(hit.left || hit.right) {
        sound_manager->play("sounds/iceblock_bump.wav", get_pos());

        if( ( dir == LEFT && hit.left ) || ( dir == RIGHT && hit.right) ){
          dir = (dir == LEFT) ? RIGHT : LEFT;
          sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");

          physic.set_velocity_x(-physic.get_velocity_x());
        }
      }
      /* fall-through */
    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      if(hit.top || hit.bottom) {
        physic.set_velocity_y(0);
      }
      break;
  }

  update_on_ground_flag(hit);

}

HitResponse
Snail::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch(state) {
    case STATE_NORMAL:
      return WalkingBadguy::collision_badguy(badguy, hit);
    case STATE_FLAT:
    case STATE_KICKED_DELAY:
      return FORCE_MOVE;
    case STATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

bool
Snail::collision_squished(GameObject& object)
{
  switch(state) {

    case STATE_KICKED:
    case STATE_NORMAL:
    {
      Player* player = dynamic_cast<Player*>(&object);
      squishcount++;
      if ((squishcount >= MAX_SNAIL_SQUISHES) || (player && player->does_buttjump)) {
        kill_fall();
        return true;
      }
    }

    sound_manager->play("sounds/stomp.wav", get_pos());
    be_flat();
    break;

    case STATE_FLAT:
      sound_manager->play("sounds/kick.wav", get_pos());
      {
        MovingObject* movingobject = dynamic_cast<MovingObject*>(&object);
        if (movingobject && (movingobject->get_pos().x < get_pos().x)) {
          dir = RIGHT;
        } else {
          dir = LEFT;
        }
      }
      be_kicked();
      break;

    case STATE_KICKED_DELAY:
      break;

  }

  Player* player = dynamic_cast<Player*>(&object);
  if (player) player->bounce(*this);
  return true;
}

/* EOF */
