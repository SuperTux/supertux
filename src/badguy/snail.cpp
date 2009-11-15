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

//#include <config.h>

#include "badguy/snail.hpp"

#include "audio/sound_manager.hpp"
#include "lisp/writer.hpp"
//#include "object/block.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"

#include <math.h>

namespace {
  const float KICKSPEED = 500;
  const int MAXSQUISHES = 10;
  const float KICKSPEED_Y = -500; /**< y-velocity gained when kicked */
}

Snail::Snail(const lisp::Lisp& reader)
  : WalkingBadguy(reader, "images/creatures/snail/snail.sprite", "left", "right"), state(STATE_NORMAL), squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
}

Snail::Snail(const Vector& pos, Direction d)
  : WalkingBadguy(pos, d, "images/creatures/snail/snail.sprite", "left", "right"), state(STATE_NORMAL), squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  sound_manager->preload("sounds/iceblock_bump.wav");
  sound_manager->preload("sounds/stomp.wav");
  sound_manager->preload("sounds/kick.wav");
}

void
Snail::write(lisp::Writer& writer)
{
  writer.start_list("snail");
  WalkingBadguy::write(writer);
  writer.end_list("snail");
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
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
  sprite->set_fps(64);

  physic.set_velocity_x(0);
  physic.set_velocity_y(0);

  flat_timer.start(4);
}

void
Snail::be_kicked()
{
  state = STATE_KICKED_DELAY;
  sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");
  sprite->set_fps(64);

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
      break;

    case STATE_FLAT:
      if (flat_timer.started()) {
        sprite->set_fps(64 - 15 * flat_timer.get_timegone());
      }
      if (flat_timer.check()) {
        be_normal();
      }
      BadGuy::active_update(elapsed_time);
      break;

    case STATE_KICKED_DELAY:
      if (kicked_delay_timer.check()) {
        physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
        physic.set_velocity_y(KICKSPEED_Y);
        state = STATE_KICKED;
      }
      BadGuy::active_update(elapsed_time);
      break;

    case STATE_KICKED:
      physic.set_velocity_x(physic.get_velocity_x() * pow(0.99, elapsed_time/0.02));
      if (fabsf(physic.get_velocity_x()) < walk_speed) be_normal();
      BadGuy::active_update(elapsed_time);
      break;

  }
}

void
Snail::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);

  switch (state) {
    case STATE_NORMAL:
      WalkingBadguy::collision_solid(hit);
      break;
    case STATE_FLAT:
      if(hit.top || hit.bottom) {
        physic.set_velocity_y(0);
      }
      if(hit.left || hit.right) {
      }
      break;
    case STATE_KICKED_DELAY:
      if(hit.top || hit.bottom) {
        physic.set_velocity_y(0);
      }
      if(hit.left || hit.right) {
        physic.set_velocity_x(0);
      }
      break;
    case STATE_KICKED:
      if(hit.top || hit.bottom) {
        physic.set_velocity_y(0);
      }
      if(hit.left || hit.right) {
        sound_manager->play("sounds/iceblock_bump.wav", get_pos());

        if( ( dir == LEFT && hit.left ) || ( dir == RIGHT && hit.right) ){
          dir = (dir == LEFT) ? RIGHT : LEFT;
          sprite->set_action(dir == LEFT ? "flat-left" : "flat-right");

          physic.set_velocity_x(-physic.get_velocity_x()*0.75);
          if (fabsf(physic.get_velocity_x()) < walk_speed) be_normal();
        }

      }
      break;
  }

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
        if ((squishcount >= MAXSQUISHES) || (player && player->does_buttjump)) {
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

IMPLEMENT_FACTORY(Snail, "snail");
