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
#include "util/gettext.hpp"
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
ScriptedObject::save(lisp::Writer& writer){
  MovingSprite::save(writer);
  writer.write("width",bbox.get_width());
  writer.write("height",bbox.get_height());
  writer.write("solid",solid);
  writer.write("physic-enabled",physic_enabled);
  writer.write("visible",visible);
}

ObjectSettings
ScriptedObject::get_settings() {
  ObjectSettings result(_("Scripted object"));
  result.options.push_back( ObjectOption(MN_TEXTFIELD, _("Name"), &name));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Solid"), &solid));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Enabled physics"), &physic_enabled));
  result.options.push_back( ObjectOption(MN_TOGGLE, _("Visible"), &visible));

  return result;
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
ScriptedObject::get_pos_x() const
{
  return get_pos().x;
}

float
ScriptedObject::get_pos_y() const
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
ScriptedObject::get_velocity_x() const
{
  return physic.get_velocity_x();
}

float
ScriptedObject::get_velocity_y() const
{
  return physic.get_velocity_y();
}

void
ScriptedObject::set_visible(bool visible_)
{
  this->visible = visible_;
}

bool
ScriptedObject::is_visible() const
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
ScriptedObject::is_solid() const
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
ScriptedObject::get_action() const
{
  return sprite->get_action();
}

std::string
ScriptedObject::get_name() const
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
