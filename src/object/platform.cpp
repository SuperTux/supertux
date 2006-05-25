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

#include "platform.hpp"

#include <stdexcept>
#include "log.hpp"
#include "video/drawing_context.hpp"
#include "resources.hpp"
#include "player.hpp"
#include "path.hpp"
#include "path_walker.hpp"
#include "sprite/sprite.hpp"
#include "lisp/lisp.hpp"
#include "object_factory.hpp"
#include "scripting/platform.hpp"
#include "scripting/squirrel_util.hpp"

Platform::Platform(const lisp::Lisp& reader)
	: MovingSprite(reader, Vector(0,0), LAYER_OBJECTS, COLGROUP_STATIC), speed(Vector(0,0))
{
  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if(pathLisp == NULL)
    throw std::runtime_error("No path specified for platform");
  path.reset(new Path());
  path->read(*pathLisp);
  walker.reset(new PathWalker(path.get()));
  bbox.set_pos(path->get_base());
 
  flags |= FLAG_SOLID;
}

Platform::Platform(const Platform& other)
	: MovingSprite(other), ScriptInterface(other), speed(other.speed)
{
  path.reset(new Path(*other.path));
  walker.reset(new PathWalker(*other.walker));
  walker->path = &*path;
}

//TODO: Squish Tux when standing between platform and solid tile/object
//      Improve collision handling
//      Move all MovingObjects lying on the platform instead of only the player
HitResponse
Platform::collision(GameObject& other, const CollisionHit& hit)
{
  if (typeid(other) == typeid(Player)) {
    if (hit.normal.y >= 0.9) {
      //Tux is standing on the platform
      //Player* player = (Player*) &other;
      //player->add_velocity(speed * 1.5);
      return PASS_MOVEMENT;
    }
  }
  if(other.get_flags() & FLAG_SOLID) {
    //Collision with a solid tile
    return ABORT_MOVE;
  }
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  movement = walker->advance(elapsed_time);
  speed = movement / elapsed_time;
}

void
Platform::goto_node(int node_no)
{
  walker->goto_node(node_no);
}

void
Platform::start_moving()
{
  walker->start_moving();
}

void
Platform::stop_moving()
{
  walker->stop_moving();
}

void
Platform::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  Scripting::Platform* interface = new Scripting::Platform(this);
  expose_object(vm, table_idx, interface, "Platform", true);
}

void
Platform::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  Scripting::unexpose_object(vm, table_idx, "Platform");
}

IMPLEMENT_FACTORY(Platform, "platform");
