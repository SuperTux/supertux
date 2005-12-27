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

#include "unstable_tile.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"
#include "player.hpp"
#include "sector.hpp"
#include "resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "sprite/sprite.hpp"

static const float CRACKTIME = 0.3;
static const float FALLTIME = 0.8;

UnstableTile::UnstableTile(const lisp::Lisp& lisp)
  : hit(false), falling(false)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("unstable_tile");
  flags |= FLAG_SOLID;
  set_group(COLGROUP_STATIC);
}

UnstableTile::~UnstableTile()
{
  delete sprite;
}

HitResponse
UnstableTile::collision(GameObject& other, const CollisionHit& hitdata)
{
  if(hitdata.normal.y < 0.8)
    return FORCE_MOVE;

  Player* player = dynamic_cast<Player*> (&other);
  if(player)
    hit = true;

  return FORCE_MOVE;
}

void
UnstableTile::draw(DrawingContext& context)
{
  Vector pos = get_pos();
  // shacking
  if(timer.get_timegone() > CRACKTIME) {
    pos.x += (rand() % 6) - 3;
  } 

  sprite->draw(context, pos, LAYER_TILES);
}

void
UnstableTile::update(float elapsed_time)
{
  if(falling) {
    movement = physic.get_movement(elapsed_time);
    if(!Sector::current()->inside(bbox)) {
      remove_me();
      return;
    }
  } else if(hit) {
    if(timer.check()) {
      falling = true;
      physic.enable_gravity(true);      
      flags &= ~FLAG_SOLID;
      timer.stop();
    } else if(!timer.started()) {
      timer.start(FALLTIME);
    }
  } else {
    timer.stop();
  }
  hit = false;
}

IMPLEMENT_FACTORY(UnstableTile, "unstable_tile");
