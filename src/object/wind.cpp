//  SuperTux - Wind
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include "wind.hpp"
#include "video/drawing_context.hpp"
#include "object/player.hpp"
#include "object_factory.hpp"
#include "random_generator.hpp"
#include "sector.hpp"
#include "particles.hpp"
#include "scripting/wind.hpp"
#include "scripting/squirrel_util.hpp"
#include "lisp/lisp.hpp"

Wind::Wind(const lisp::Lisp& reader)
  : blowing(true), acceleration(100), elapsed_time(0)
{
  reader.get("name", name);
  reader.get("x", bbox.p1.x);
  reader.get("y", bbox.p1.y);
  float w = 32, h = 32;
  reader.get("width", w);
  reader.get("height", h);
  bbox.set_size(w, h);

  reader.get("blowing", blowing);

  float speed_x = 0, speed_y = 0;
  reader.get("speed-x", speed_x);
  reader.get("speed-y", speed_y);
  speed = Vector(speed_x, speed_y);

  reader.get("acceleration", acceleration);

  set_group(COLGROUP_TOUCHABLE);
}

void
Wind::update(float elapsed_time)
{
  this->elapsed_time = elapsed_time;

  if (!blowing) return;

  // TODO: nicer, configurable particles for wind?
  if (systemRandom.rand(0, 100) < 20) {
    // emit a particle
    Vector ppos = Vector(systemRandom.randf(bbox.p1.x+8, bbox.p2.x-8), systemRandom.randf(bbox.p1.y+8, bbox.p2.y-8));
    Vector pspeed = Vector(speed.x, speed.y);
    Sector::current()->add_object(new Particles(ppos, 44, 46, pspeed, Vector(0,0), 1, Color(.4f, .4f, .4f), 3, .1f, LAYER_BACKGROUNDTILES+1));
  }
}

void
Wind::draw(DrawingContext& )
{
}

HitResponse
Wind::collision(GameObject& other, const CollisionHit& )
{
  if (!blowing) return ABORT_MOVE;

  Player* player = dynamic_cast<Player*> (&other);
  if (player) {
    if (!player->on_ground()) {
      player->add_velocity(speed * acceleration * elapsed_time, speed);
    }
  }

  return ABORT_MOVE;
}

void
Wind::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name == "")
    return;

  Scripting::Wind* interface = new Scripting::Wind(this);
  expose_object(vm, table_idx, interface, name, true);
}

void
Wind::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name == "")
    return;

  Scripting::unexpose_object(vm, table_idx, name);
}

void
Wind::start()
{
  blowing = true;
}

void
Wind::stop()
{
  blowing = false;
}

IMPLEMENT_FACTORY(Wind, "wind");
