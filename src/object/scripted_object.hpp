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
#include "squirrel/exposed_object.hpp"
#include "supertux/physic.hpp"

class ScriptedObject final :
  public MovingSprite,
  public ExposedObject<ScriptedObject, scripting::ScriptedObject>
{
public:
  ScriptedObject(const ReaderMapping& mapping);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  virtual std::string get_class() const override { return "scriptedobject"; }
  virtual std::string get_display_name() const override { return _("Scripted Object"); }

  virtual ObjectSettings get_settings() override;

  virtual void on_flip(float height) override;

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

private:
  Physic physic;
  bool solid;
  bool physic_enabled;
  bool visible;
  std::string hit_script;
  bool new_vel_set;
  Vector new_vel;
  Vector new_size;

private:
  ScriptedObject(const ScriptedObject&) = delete;
  ScriptedObject& operator=(const ScriptedObject&) = delete;
};

#endif

/* EOF */
