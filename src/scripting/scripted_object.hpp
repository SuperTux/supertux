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

#ifndef HEADER_SUPERTUX_SCRIPTING_SCRIPTED_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SCRIPTED_OBJECT_HPP

#ifndef SCRIPTING_API
#include "scripting/sqrat_object.hpp"
#include <string>

class ScriptedObject;
#endif

namespace scripting {

#ifdef SCRIPTING_API
class ScriptedObject
#else
class ScriptedObject: SQRatObject<ScriptedObject>
#endif
{
#ifndef SCRIPTING_API
private:
  ::ScriptedObject* m_parent;

public:
  ScriptedObject(::ScriptedObject* parent);
  ~ScriptedObject();
  static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
  {
    squirrelClass.Func("set_action", &ScriptedObject::set_action);
    squirrelClass.Func("get_action", &ScriptedObject::get_action);
    squirrelClass.Func("move", &ScriptedObject::move);
    squirrelClass.Func("set_pos", &ScriptedObject::set_pos);
    squirrelClass.Func("get_pos_x", &ScriptedObject::get_pos_x);
    squirrelClass.Func("get_pos_y", &ScriptedObject::get_pos_y);
    squirrelClass.Func("set_velocity", &ScriptedObject::set_velocity);
    squirrelClass.Func("get_velocity_x", &ScriptedObject::get_velocity_x);
    squirrelClass.Func("get_velocity_y", &ScriptedObject::get_velocity_y);
    squirrelClass.Func("enable_gravity", &ScriptedObject::enable_gravity);
    squirrelClass.Func("gravity_enabled", &ScriptedObject::gravity_enabled);
    squirrelClass.Func("set_visible", &ScriptedObject::set_visible);
    squirrelClass.Func("is_visible", &ScriptedObject::is_visible);
    squirrelClass.Func("set_solid", &ScriptedObject::set_solid);
    squirrelClass.Func("is_solid", &ScriptedObject::is_solid);
    squirrelClass.Func("get_name", &ScriptedObject::get_name);
  }

private:
  ScriptedObject(const ScriptedObject&) = delete;
  ScriptedObject& operator=(const ScriptedObject&) = delete;
#endif

public:
  void set_action(const std::string& animation);
  std::string get_action() const;

  void move(float x, float y);
  void set_pos(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;

  void set_velocity(float x, float y);
  float get_velocity_x() const;
  float get_velocity_y() const;

  void enable_gravity(bool f);
  bool gravity_enabled() const;

  void set_visible(bool visible);
  bool is_visible() const;

  void set_solid(bool solid);
  bool is_solid() const;

  std::string get_name() const;
};

} // namespace scripting

#endif

/* EOF */
