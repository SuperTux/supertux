//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <config.h>

#include <math.h>
#include <stdexcept>

#include "math/vector.hpp"
#include "object/scripted_object.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "supertux/resources.hpp"
#include "video/drawing_context.hpp"

ScriptedObject::ScriptedObject(const lisp::Lisp& lisp)
  : MovingSprite(lisp, LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
    solid(true), physic_enabled(true), visible(true), new_vel_set(false)
{
  lisp.get("name", name);
  if(name == "")
    throw std::runtime_error("Scripted object must have a name specified");

  // FIXME: do we need this? bbox is already set via .sprite file
  float width = sprite->get_width();
  float height = sprite->get_height();
  lisp.get("width", width);
  lisp.get("height", height);
  bbox.set_size(width, height);

  lisp.get("solid", solid);
  lisp.get("physic-enabled", physic_enabled);
  lisp.get("visible", visible);
  lisp.get("z-pos", layer);
  if( solid ){
    set_group( COLGROUP_MOVING_STATIC );
  } else {
    set_group( COLGROUP_DISABLED );
  }
}

void
ScriptedObject::expose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  expose_object(vm, table_idx, dynamic_cast<Scripting::ScriptedObject *>(this), name, false);
}

void
ScriptedObject::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  Scripting::unexpose_object(vm, table_idx, name);
}

void
ScriptedObject::move(float x, float y)
{
  bbox.move(Vector(x, y));
}

void
ScriptedObject::set_pos(float x, float y)
{
  printf("SetPos: %f %f\n", x, y);
  bbox.set_pos(Vector(x, y));
  physic.reset();
}

float
ScriptedObject::get_pos_x()
{
  return get_pos().x;
}

float
ScriptedObject::get_pos_y()
{
  return get_pos().y;
}

void
ScriptedObject::set_velocity(float x, float y)
{
  new_vel = Vector(x, y);
  new_vel_set = true;
}

float
ScriptedObject::get_velocity_x()
{
  return physic.get_velocity_x();
}

float
ScriptedObject::get_velocity_y()
{
  return physic.get_velocity_y();
}

void
ScriptedObject::set_visible(bool visible)
{
  this->visible = visible;
}

bool
ScriptedObject::is_visible()
{
  return visible;
}

void
ScriptedObject::set_solid(bool solid)
{
  this->solid = solid;
  if( solid ){
    set_group( COLGROUP_MOVING_STATIC );
  } else {
    set_group( COLGROUP_DISABLED );
  }
}

bool
ScriptedObject::is_solid()
{
  return solid;
}


void
ScriptedObject::set_action(const std::string& animation)
{
  sprite->set_action(animation);
}

std::string
ScriptedObject::get_action()
{
  return sprite->get_action();
}

std::string
ScriptedObject::get_name()
{
  return name;
}

void
ScriptedObject::update(float elapsed_time)
{
  if(!physic_enabled)
    return;

  if(new_vel_set) {
    physic.set_velocity(new_vel.x, new_vel.y);
    new_vel_set = false;
  }
  movement = physic.get_movement(elapsed_time);
}

void
ScriptedObject::draw(DrawingContext& context)
{
  if(!visible)
    return;

  sprite->draw(context, get_pos(), layer);
}

void
ScriptedObject::collision_solid(const CollisionHit& hit)
{
  if(!physic_enabled)
    return;

  if(hit.bottom) {
    if(physic.get_velocity_y() > 0)
      physic.set_velocity_y(0);
  } else if(hit.top) {
    physic.set_velocity_y(.1f);
  }

  if(hit.left || hit.right) {
    physic.set_velocity_x(0);
  }
}

HitResponse
ScriptedObject::collision(GameObject& , const CollisionHit& )
{
  return FORCE_MOVE;
}

IMPLEMENT_FACTORY(ScriptedObject, "scriptedobject");
