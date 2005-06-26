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

#include "oneup.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "player_status.hpp"
#include "sector.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"

OneUp::OneUp(const Vector& pos)
{
  bbox.set_pos(pos);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("1up");
  physic.set_velocity(100, 400);
}

OneUp::~OneUp()
{
  delete sprite;
}

void
OneUp::update(float elapsed_time)
{
  if(!Sector::current()->inside(bbox))
    remove_me();

  movement = physic.get_movement(elapsed_time); 
}

void
OneUp::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

HitResponse
OneUp::collision(GameObject& other, const CollisionHit& )
{
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    player->get_status()->incLives();
    remove_me();
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

