//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "rock.hpp"
#include "sprite/sprite.hpp"
#include "sprite/sprite_manager.hpp"
#include "lisp/writer.hpp"
#include "video/drawing_context.hpp"
#include "resources.hpp"
#include "object_factory.hpp"

Rock::Rock(const lisp::Lisp& reader)
{
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  bbox.set_size(31.8, 31.8);
  sprite = sprite_manager->create("images/objects/rock/rock.sprite");
  grabbed = false;
  flags |= FLAG_SOLID | FLAG_PORTABLE;
  set_group(COLGROUP_MOVING);
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
    set_group(COLGROUP_MOVING);
    movement = physic.get_movement(elapsed_time);
  } else {
    physic.set_velocity(0, 0);
    flags &= ~FLAG_SOLID;
    set_group(COLGROUP_DISABLED);
  }
  
  grabbed = false;
  printf("%p - V %3.1f %3.1f - P %3.1f %3.1f\n", this,
          physic.get_velocity().x, physic.get_velocity().y,
          get_pos().x, get_pos().y);
}

HitResponse
Rock::collision(GameObject& object, const CollisionHit& hit)
{
  if(grabbed) {
    return FORCE_MOVE;
  }

  if(object.get_flags() & FLAG_SOLID) {
    printf("%p vs %p - %3.1f %3.1f D %3.1f\n", this, &object,
        hit.normal.x, hit.normal.y, hit.depth);
    physic.set_velocity(0, 0);
    return CONTINUE;
  }

  return FORCE_MOVE;
}

void
Rock::grab(MovingObject& , const Vector& pos, Direction)
{
  movement = pos - get_pos();
  grabbed = true;
}

IMPLEMENT_FACTORY(Rock, "rock");

