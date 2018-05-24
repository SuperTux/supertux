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

#include "badguy/mriceblock.hpp"

#include "audio/sound_manager.hpp"
#include "object/player.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/sector.hpp"

#include <math.h>

namespace {
const float KICKSPEED = 500;
const int MAXSQUISHES = 10;
const float NOKICK_TIME = 0.1f;
}

MrIceBlock::MrIceBlock(const ReaderMapping& reader) :
  WalkingBadguy(reader, "images/creatures/mr_iceblock/mr_iceblock.sprite", "left", "right"),
  ice_state(ICESTATE_NORMAL),
  nokick_timer(),
  flat_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  SoundManager::current()->preload("sounds/iceblock_bump.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
  SoundManager::current()->preload("sounds/kick.wav");
}

MrIceBlock::MrIceBlock(const Vector& pos, Direction d) :
  WalkingBadguy(pos, d, "images/creatures/mr_iceblock/mr_iceblock.sprite", "left", "right"),
  ice_state(ICESTATE_NORMAL),
  nokick_timer(),
  flat_timer(),
  squishcount(0)
{
  walk_speed = 80;
  max_drop_height = 600;
  SoundManager::current()->preload("sounds/iceblock_bump.wav");
  SoundManager::current()->preload("sounds/stomp.wav");
  SoundManager::current()->preload("sounds/kick.wav");
}

void
MrIceBlock::initialize()
{
  WalkingBadguy::initialize();
  set_state(ICESTATE_NORMAL);
}

void
MrIceBlock::active_update(float elapsed_time)
{
  if(ice_state == ICESTATE_GRABBED)
    return;

  if(ice_state == ICESTATE_FLAT && flat_timer.check()) {
    set_state(ICESTATE_WAKING);
  }

  if (ice_state == ICESTATE_WAKING && sprite->animation_done()) {
    set_state(ICESTATE_NORMAL);
  }

  if (ice_state == ICESTATE_NORMAL)
  {
    WalkingBadguy::active_update(elapsed_time);
    return;
  }

  BadGuy::active_update(elapsed_time);
}

bool
MrIceBlock::can_break() const {
  return ice_state == ICESTATE_KICKED;
}

void
MrIceBlock::collision_solid(const CollisionHit& hit)
{
  update_on_ground_flag(hit);

  if(hit.top || hit.bottom) { // floor or roof
    physic.set_velocity_y(0);
  }

  // hit left or right
  switch(ice_state) {
    case ICESTATE_NORMAL:
      WalkingBadguy::collision_solid(hit);
      break;
    case ICESTATE_KICKED: {
      if((hit.right && dir == RIGHT) || (hit.left && dir == LEFT)) {
        dir = (dir == LEFT) ? RIGHT : LEFT;
        SoundManager::current()->play("sounds/iceblock_bump.wav", get_pos());
        physic.set_velocity_x(-physic.get_velocity_x()*.975);
      }
      this->set_action(dir == LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      if(fabsf(physic.get_velocity_x()) < walk_speed*1.5)
        set_state(ICESTATE_NORMAL);
      break;
    }
    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
      physic.set_velocity_x(0);
      break;
    case ICESTATE_GRABBED:
      break;
  }
}

HitResponse
MrIceBlock::collision(GameObject& object, const CollisionHit& hit)
{
  if(ice_state == ICESTATE_GRABBED)
    return FORCE_MOVE;

  return BadGuy::collision(object, hit);
}

HitResponse
MrIceBlock::collision_player(Player& player, const CollisionHit& hit)
{
  // handle kicks from left or right side
  if(ice_state == ICESTATE_FLAT && get_state() == STATE_ACTIVE) {
    if(hit.left) {
      dir = RIGHT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    } else if(hit.right) {
      dir = LEFT;
      player.kick();
      set_state(ICESTATE_KICKED);
      return FORCE_MOVE;
    }
  }

  return BadGuy::collision_player(player, hit);
}

HitResponse
MrIceBlock::collision_badguy(BadGuy& badguy, const CollisionHit& hit)
{
  switch(ice_state) {
    case ICESTATE_NORMAL:
      return WalkingBadguy::collision_badguy(badguy, hit);
    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
      return FORCE_MOVE;
    case ICESTATE_KICKED:
      badguy.kill_fall();
      return FORCE_MOVE;
    default:
      assert(false);
  }

  return ABORT_MOVE;
}

bool
MrIceBlock::collision_squished(GameObject& object)
{
  Player* player = dynamic_cast<Player*>(&object);
  if(player && (player->does_buttjump || player->is_invincible())) {
    player->bounce(*this);
    kill_fall();
    return true;
  }

  switch(ice_state) {
    case ICESTATE_KICKED:
    {
      auto badguy = dynamic_cast<BadGuy*>(&object);
      if (badguy) {
        badguy->kill_fall();
        break;
      }
    }

    // fall through
    case ICESTATE_NORMAL:
    {
      squishcount++;
      if (squishcount >= MAXSQUISHES) {
        kill_fall();
        return true;
      }
    }

    set_state(ICESTATE_FLAT);
    nokick_timer.start(NOKICK_TIME);
    break;
    case ICESTATE_FLAT:
    case ICESTATE_WAKING:
    {
      auto movingobject = dynamic_cast<MovingObject*>(&object);
      if (movingobject && (movingobject->get_pos().x < get_pos().x)) {
        dir = RIGHT;
      } else {
        dir = LEFT;
      }
    }
    if (nokick_timer.check()) set_state(ICESTATE_KICKED);
    break;
    case ICESTATE_GRABBED:
      assert(false);
      break;
  }

  if (player) player->bounce(*this);
  return true;
}

void
MrIceBlock::set_state(IceState state_, bool up)
{
  if(ice_state == state_)
    return;

  switch(state_) {
    case ICESTATE_NORMAL:
      this->set_action(dir == LEFT ? "left" : "right", /* loops = */ -1);
      WalkingBadguy::initialize();
      break;
    case ICESTATE_FLAT:
      if(up) {
        physic.set_velocity_y(-KICKSPEED);
      } else {
        SoundManager::current()->play("sounds/stomp.wav", get_pos());
        physic.set_velocity_x(0);
        physic.set_velocity_y(0);
      }
      this->set_action(dir == LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      flat_timer.start(4);
      break;
    case ICESTATE_KICKED:
      SoundManager::current()->play("sounds/kick.wav", get_pos());

      physic.set_velocity_x(dir == LEFT ? -KICKSPEED : KICKSPEED);
      this->set_action(dir == LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
      // we should slide above 1 block holes now...
      bbox.set_size(34, 31.8f);
      break;
    case ICESTATE_GRABBED:
      flat_timer.stop();
      break;
    case ICESTATE_WAKING:
      sprite->set_action(dir == LEFT ? "waking-left" : "waking-right",
                         /* loops = */ 1);
      break;
    default:
      assert(false);
  }
  ice_state = state_;
}

void
MrIceBlock::grab(MovingObject&, const Vector& pos, Direction dir_)
{
  movement = pos - get_pos();
  this->dir = dir_;
  this->set_action(dir_ == LEFT ? "flat-left" : "flat-right", /* loops = */ -1);
  set_state(ICESTATE_GRABBED);
  set_colgroup_active(COLGROUP_DISABLED);
}

void
MrIceBlock::ungrab(MovingObject& , Direction dir_)
{
  if(dir_ == UP) {
    set_state(ICESTATE_FLAT, true);
  } else {
    this->dir = dir_;
    set_state(ICESTATE_KICKED);
  }
  set_colgroup_active(COLGROUP_MOVING);
}

bool
MrIceBlock::is_portable() const
{
  return ice_state == ICESTATE_FLAT;
}

void
MrIceBlock::ignite() {
  set_state(ICESTATE_NORMAL);
  BadGuy::ignite();
}

SmartBlock::SmartBlock(const ReaderMapping& reader) :
  MrIceBlock(reader)
{
  max_drop_height = 16;
  sprite = SpriteManager::current()->create("images/creatures/mr_iceblock/smart_block/smart_block.sprite");
}

/* EOF */
