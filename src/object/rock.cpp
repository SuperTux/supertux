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

#include "rock.h"
#include "sprite/sprite.h"
#include "sprite/sprite_manager.h"
#include "lisp/writer.h"
#include "video/drawing_context.h"
#include "resources.h"
#include "object_factory.h"

Rock::Rock(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("rock");
  grabbed = false;
  flags |= FLAG_SOLID;
}

Rock::~Rock()
{
  delete sprite;
}

void
Rock::write(lisp::Writer& writer)
{
  writer.start_list("rock");

  writer.write_float("x", bbox.p1.x);
  writer.write_float("y", bbox.p1.y);

  writer.end_list("rock");
}

void
Rock::draw(DrawingContext& context)
{

  sprite->draw(context, get_pos(), LAYER_OBJECTS);
}

void
Rock::update(float elapsed_time)
{
  if(!grabbed) {
    flags |= FLAG_SOLID;
    flags &= ~FLAG_NO_COLLDET;
    movement = physic.get_movement(elapsed_time);
  } else {
    physic.set_velocity(0, 0);
    flags &= ~FLAG_SOLID;
    flags |= FLAG_NO_COLLDET;
  }
  
  grabbed = false;
}

HitResponse
Rock::collision(GameObject& object, const CollisionHit& )
{
  if(grabbed)
    return FORCE_MOVE;

  if(object.get_flags() & FLAG_SOLID) {
      physic.set_velocity(0, 0);
      return CONTINUE;
  }

  return FORCE_MOVE;
}

void
Rock::grab(MovingObject& , const Vector& pos)
{
  movement = pos - get_pos();
  grabbed = true;
}

IMPLEMENT_FACTORY(Rock, "rock");

