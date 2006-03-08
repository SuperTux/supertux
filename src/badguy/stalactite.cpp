//  $Id$
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include "stalactite.hpp"

static const int SHAKE_RANGE = 40;
static const float SHAKE_TIME = .8;
static const float SQUISH_TIME = 2;

Stalactite::Stalactite(const lisp::Lisp& lisp)
{
  lisp.get("x", start_position.x);
  lisp.get("y", start_position.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/creatures/stalactite/stalactite.sprite");
  state = STALACTITE_HANGING;
  countMe = false;
}

void
Stalactite::write(lisp::Writer& writer)
{
  writer.start_list("stalactite");
  writer.write_float("x", start_position.x);
  writer.write_float("y", start_position.y);
  writer.end_list("stalactite");
}

void
Stalactite::active_update(float elapsed_time)
{
  if(state == STALACTITE_HANGING) {
    Player* player = Sector::current()->player;
    if(player->get_bbox().p2.x > bbox.p1.x - SHAKE_RANGE
        && player->get_bbox().p1.x < bbox.p2.x + SHAKE_RANGE
        && player->get_bbox().p2.y > bbox.p1.y) {
      timer.start(SHAKE_TIME);
      state = STALACTITE_SHAKING;
    }
  } else if(state == STALACTITE_SHAKING) {
    if(timer.check()) {
      state = STALACTITE_FALLING;
      physic.enable_gravity(true);
    }
  } else if(state == STALACTITE_FALLING || state == STALACTITE_SQUISHED) {
    movement = physic.get_movement(elapsed_time);
    if(state == STALACTITE_SQUISHED && timer.check())
      remove_me();
  }
}

HitResponse
Stalactite::collision_solid(GameObject& , const CollisionHit& hit)
{
  if(state != STALACTITE_FALLING && state != STALACTITE_SQUISHED)
    return FORCE_MOVE;
  
  if(fabsf(hit.normal.y) > .5) { // hit floor or roof?
    state = STALACTITE_SQUISHED;
    set_group(COLGROUP_MOVING_ONLY_STATIC);
    physic.set_velocity_y(0);
    sprite->set_action("squished");
    if(!timer.started())
      timer.start(SQUISH_TIME);
  }

  return CONTINUE;
}

HitResponse
Stalactite::collision_player(Player& player, const CollisionHit& )
{
  if(state != STALACTITE_SQUISHED) {
    player.kill(Player::SHRINK);
  }

  return FORCE_MOVE;
}

void
Stalactite::kill_fall()
{
}

void
Stalactite::draw(DrawingContext& context)
{
  if(get_state() != STATE_ACTIVE)
    return;
    
  if(state == STALACTITE_SHAKING) {
    sprite->draw(context, get_pos() + Vector((rand() % 6)-3, 0), LAYER_OBJECTS);
  } else {
    sprite->draw(context, get_pos(), LAYER_OBJECTS);
  }
}

void
Stalactite::deactivate()
{
  if(state != STALACTITE_HANGING)
    remove_me();
}

IMPLEMENT_FACTORY(Stalactite, "stalactite")
