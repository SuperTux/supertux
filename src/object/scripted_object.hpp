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
#include "supertux/physic.hpp"
#include "supertux/script_interface.hpp"

class ScriptedObject : public MovingSprite,
                       public ScriptInterface
{
public:
  ScriptedObject(const ReaderMapping& lisp);

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

  void update(float elapsed_time);
  void draw(DrawingContext& context);

  void collision_solid(const CollisionHit& hit);
  HitResponse collision(GameObject& other, const CollisionHit& hit);

  // --- scripting Interface stuff ---

  void set_action(const std::string& animation);
  std::string get_action() const;

  void move(float x, float y);
  float get_pos_x() const;
  float get_pos_y() const;
  void set_velocity(float x, float y);
  float get_velocity_x() const;
  float get_velocity_y() const;
  void set_visible(bool visible);
  bool is_visible() const;
  void set_solid(bool solid);
  bool is_solid() const;
  void enable_gravity(bool f);
  bool gravity_enabled() const;

  std::string get_name() const;

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
