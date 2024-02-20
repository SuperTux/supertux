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

class ScriptedObject final : public MovingSprite
{
public:
  static void register_class(ssq::VM& vm);

public:
  ScriptedObject(const ReaderMapping& mapping);

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual void collision_solid(const CollisionHit& hit) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "scriptedobject"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "ScriptedObject"; }
  static std::string display_name() { return _("Scripted Object"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;

  virtual void on_flip(float height) override;

#ifdef DOXYGEN_SCRIPTING
  /**
   * @deprecated Use ""get_x()"" instead!
   * Returns the X coordinate of the object's position.
   */
  float get_pos_x() const;
  /**
   * @deprecated Use ""get_y()"" instead!
   * Returns the Y coordinate of the object's position.
   */
  float get_pos_y() const;
#endif

  /**
   * Makes the object move in a certain ""x"" and ""y"" direction (with a certain speed).
   * @param float $x
   * @param float $y
   */
  void set_velocity(float x, float y);
  /**
   * Returns the X coordinate of the object's velocity.
   */
  float get_velocity_x() const;
  /**
   * Returns the Y coordinate of the object's velocity.
   */
  float get_velocity_y() const;

  /**
   * Enables or disables gravity, according to the value of ""enabled"".
   * @param bool $enabled
   */
  void enable_gravity(bool enabled);
  /**
   * Returns ""true"" if the object's gravity is enabled.
   */
  bool gravity_enabled() const;

  /**
   * @deprecated Use the ""visible"" property instead!
   * Shows or hides the object, according to the value of ""visible"".
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
   * @deprecated Use the ""visible"" property instead!
   * Returns ""true"" if the object is visible.
   */
  bool is_visible() const;

  /**
   * Changes the solidity, according to the value of ""solid"".
   * @param bool $solid
   */
  void set_solid(bool solid);
  /**
   * Returns ""true"" if the object is solid.
   */
  bool is_solid() const;

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
