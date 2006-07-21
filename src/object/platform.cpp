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
	: MovingSprite(reader, Vector(0,0), LAYER_OBJECTS, COLGROUP_STATIC), name(""), speed(Vector(0,0))
{
  bool running = true;
  reader.get("name", name);
  reader.get("running", running);
  const lisp::Lisp* pathLisp = reader.get_lisp("path");
  if(pathLisp == NULL)
    throw std::runtime_error("No path specified for platform");
  path.reset(new Path());
  path->read(*pathLisp);
  walker.reset(new PathWalker(path.get(), running));
  bbox.set_pos(path->get_base());

  flags |= FLAG_SOLID;
}

Platform::Platform(const Platform& other)
	: MovingSprite(other), ScriptInterface(other), name(other.name), speed(other.speed)
{
  path.reset(new Path(*other.path));
  walker.reset(new PathWalker(*other.walker));
  walker->path = &*path;
}

HitResponse
Platform::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

void
Platform::update(float elapsed_time)
{
  movement = walker->advance(elapsed_time) - get_pos();
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
  if (name == "") return;
  Scripting::Platform* interface = new Scripting::Platform(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
Platform::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name == "") return;
  Scripting::unexpose_object(vm, table_idx, name);
}

IMPLEMENT_FACTORY(Platform, "platform");
