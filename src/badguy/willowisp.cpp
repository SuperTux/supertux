//  $Id: willowisp.cpp 3117 2006-03-25 17:19:54Z sommer $
// 
//  SuperTux - "Will-O-Wisp" Badguy
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#include <config.h>

#include "willowisp.hpp"
#include "msg.hpp"
#include "game_session.hpp"

static const float FLYSPEED = 64; /**< speed in px per second */
static const float TRACK_RANGE = 384; /**< at what distance to start tracking the player */
static const float VANISH_RANGE = 512; /**< at what distance to stop tracking and vanish */

WillOWisp::WillOWisp(const lisp::Lisp& reader)
  : mystate(STATE_IDLE), target_sector("main"), target_spawnpoint("main"), soundSource(0)
{
  reader.get("x", start_position.x);
  reader.get("y", start_position.y);
  reader.get("sector", target_sector);
  reader.get("spawnpoint", target_spawnpoint);

  bbox.set_size(32, 32);  
  sprite = sprite_manager->create("images/creatures/willowisp/willowisp.sprite");
  countMe = false;
}

WillOWisp::~WillOWisp()
{
  delete soundSource;
}

void
WillOWisp::write(lisp::Writer& writer)
{
  writer.start_list("willowisp");

  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.write_string("sector", target_sector);
  writer.write_string("spawnpoint", target_spawnpoint);

  writer.end_list("willowisp");
}

void
WillOWisp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  
  context.push_target();
  context.set_target(DrawingContext::LIGHTMAP);

  sprite->draw(context, get_pos(), LAYER_OBJECTS);
  
  context.pop_target();
}

void
WillOWisp::active_update(float elapsed_time)
{
  Player* player = get_nearest_player();
  if (!player) return;
  Vector p1 = this->get_pos() + (this->get_bbox().p2 - this->get_bbox().p1) / 2;
  Vector p2 = player->get_pos() + (player->get_bbox().p2 - player->get_bbox().p1) / 2;
  Vector dist = (p2 - p1);

  if (mystate == STATE_IDLE) {
    if (dist.norm() <= TRACK_RANGE) {
      mystate = STATE_TRACKING;
    }
  }
  
  if (mystate == STATE_TRACKING) {
    if (dist.norm() <= VANISH_RANGE) {
      Vector dir = dist.unit();
      movement = dir*elapsed_time*FLYSPEED;
    } else {
      mystate = STATE_VANISHING;
      sprite->set_action("vanishing", 1);
    }
    soundSource->set_position(get_pos());
  }

  if (mystate == STATE_WARPING) {
    if(sprite->animation_done()) {
      remove_me();
    }
  }

  if (mystate == STATE_VANISHING) {
    if(sprite->animation_done()) {
      remove_me();
    }
  }
  
}

void
WillOWisp::activate()
{
  sprite->set_action("idle");

  delete soundSource;
  soundSource = sound_manager->create_sound_source("sounds/rain.wav");
  if(!soundSource) {
    msg_warning("Couldn't start WillOWisp sound");
    return;
  }
  soundSource->set_position(get_pos());
  soundSource->set_looping(true);
  soundSource->set_gain(2.0);
  soundSource->set_reference_distance(32);
  soundSource->play();
}

void
WillOWisp::deactivate()
{
  delete soundSource;
  soundSource = 0;
}

void
WillOWisp::kill_fall()
{
}

HitResponse
WillOWisp::collision_player(Player& player, const CollisionHit& ) {
  if(player.is_invincible()) return ABORT_MOVE;

  if (mystate != STATE_TRACKING) return ABORT_MOVE;

  mystate = STATE_WARPING;
  sprite->set_action("warping", 1);

  GameSession::current()->respawn(target_sector, target_spawnpoint);
  sound_manager->play("sounds/warp.wav");

  return CONTINUE;
}

IMPLEMENT_FACTORY(WillOWisp, "willowisp")

