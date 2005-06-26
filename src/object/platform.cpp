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

#include "platform.hpp"
#include "video/drawing_context.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "sprite/sprite_manager.hpp"
#include "lisp/lisp.hpp"
#include "lisp/writer.hpp"
#include "object_factory.hpp"

Platform::Platform(const lisp::Lisp& reader)
{
  sprite = sprite_manager->create("flying_platform");
  movement = Vector(0, 1);
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  bbox.set_size(sprite->get_width(), sprite->get_height());

  flags |= FLAG_SOLID;

  state = 0;
}

Platform::~Platform()
{
  delete sprite;
}

HitResponse
Platform::collision(GameObject& , const CollisionHit& )
{
#if 0
  if(typeid(object) == typeid(Player)) {
    Player* player = (Player*) &object;
    //player->movement += movement;
  }
#endif
  return FORCE_MOVE;
}

void
Platform::update(float )
{
  // just some test code...
  if(state == 0) {
    movement = Vector(0, 1);
    if(bbox.p1.y > 250)
      state = 1;
  }
  if(state == 1) {
    movement = Vector(0, -1);
    if(bbox.p1.y < 50)
      state = 2;
  }
  if(state == 2) {
    movement = Vector(1, 0);
    if(bbox.p1.x > 800)
      state = 3;
  }
  if(state == 3) {
    movement = Vector(-1, 0);
    if(bbox.p1.x < 400)
      state = 4;
  }
  if(state == 4) {
    movement = Vector(-1, 1);
    if(bbox.p1.x < 0)
      state = 0;
  }
}

void
Platform::draw(DrawingContext& context)
{
  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

IMPLEMENT_FACTORY(Platform, "flying_platform");
