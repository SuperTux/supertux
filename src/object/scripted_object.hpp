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

#ifndef HEADER_SUPERTUX_OBJECT_SCRIPTED_OBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_SCRIPTED_OBJECT_HPP

#include "object/moving_sprite.hpp"
#include "scripting/scripted_object.hpp"
#include "supertux/physic.hpp"
#include "supertux/script_interface.hpp"

class ScriptedObject : public MovingSprite, 
                       public Scripting::ScriptedObject, 
                       public ScriptInterface
{
public:
  ScriptedObject(const Reader& lisp);

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void update(float elapsed_time);
  void draw(DrawingContext& context);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  // --- Scripting Interface stuff ---

  void set_action(const std::string& animation);
  std::string get_action();

  void move(float x, float y);
  void set_pos(float x, float y);
  float get_pos_x();
  float get_pos_y();
  void set_velocity(float x, float y);
  float get_velocity_x();
  float get_velocity_y();
  void set_visible(bool visible);
  bool is_visible();
  void set_solid(bool solid);
  bool is_solid();

  std::string get_name();

private:
  Physic physic;
  std::string name;
  bool solid;
  bool physic_enabled;
  bool visible;
  bool new_vel_set;
  Vector new_vel;
};

#endif

/* EOF */
