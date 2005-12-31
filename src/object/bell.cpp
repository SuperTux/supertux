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

#include "bell.hpp"
#include "resources.hpp"
#include "sprite/sprite_manager.hpp"
#include "video/drawing_context.hpp"
#include "player.hpp"
#include "object_factory.hpp"
#include "game_session.hpp"
#include "sector.hpp"

Bell::Bell(const lisp::Lisp& lisp)
  : ringing(false)
{
  lisp.get("x", bbox.p1.x);
  lisp.get("y", bbox.p1.y);
  bbox.set_size(32, 32);
  sprite = sprite_manager->create("bell");
  set_group(COLGROUP_TOUCHABLE);
}

Bell::~Bell()
{
  delete sprite;
}

void
Bell::write(lisp::Writer& writer)
{
  writer.start_list("bell");
  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);
  writer.end_list("bell");
}

void
Bell::update(float )
{
}

void
Bell::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_TILES);
}

HitResponse
Bell::collision(GameObject& other, const CollisionHit& )
{
  if(ringing)
    return ABORT_MOVE;
  
  Player* player = dynamic_cast<Player*> (&other);
  if(player) {
    ringing = true;
    // TODO play sound
    sprite->set_action("ringing");
    GameSession::current()->set_reset_point(Sector::current()->get_name(),
        get_pos());
  }
  
  return ABORT_MOVE;
}

IMPLEMENT_FACTORY(Bell, "bell");
