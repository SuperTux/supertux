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

#include "object/scripted_object.hpp"

#include <stdio.h>

#include "math/random_generator.hpp"
#include "scripting/scripted_object.hpp"
#include "scripting/squirrel_util.hpp"
#include "sprite/sprite.hpp"
#include "supertux/object_factory.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"
#include "util/reader_mapping.hpp"

ScriptedObject::ScriptedObject(const ReaderMapping& lisp) :
  MovingSprite(lisp, LAYER_OBJECTS, COLGROUP_MOVING_STATIC),
  physic(),
  name(),
  solid(),
  physic_enabled(),
  visible(),
  new_vel_set(false),
  new_vel()
{
  if (!lisp.get("name", name)) name = "";
  if(name.empty()) {
    name = "unnamed" + std::to_string(graphicsRandom.rand());
    log_warning << "Scripted object must have a name specified, setting to: " << name << std::endl;
  }

  if (!lisp.get("solid", solid)) solid = true;
  if (!lisp.get("physic-enabled", physic_enabled)) physic_enabled = true;
  if (!lisp.get("visible", visible)) visible = true;
  layer = reader_get_layer (lisp, /* default = */ LAYER_OBJECTS);
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
  auto obj = new scripting::ScriptedObject(this);
  expose_object(vm, table_idx, obj, name, true);
}

void
ScriptedObject::unexpose(HSQUIRRELVM vm, SQInteger table_idx)
{
  if (name.empty()) return;
  scripting::unexpose_object(vm, table_idx, name);
}

void
ScriptedObject::move(float x, float y)
{
  bbox.move(Vector(x, y));
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
ScriptedObject::set_visible(bool visible_)
{
  this->visible = visible_;
}

bool
ScriptedObject::is_visible()
{
  return visible;
}

void
ScriptedObject::set_solid(bool solid_)
{
  this->solid = solid_;
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

bool
ScriptedObject::gravity_enabled() const
{
	return physic.gravity_enabled();
}

void
ScriptedObject::enable_gravity(bool f)
{
	physic.enable_gravity(f);
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

/* EOF */
