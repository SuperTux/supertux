//  SuperTux
//  Copyright (C) 2022 Raoul1808 <raoulthegeek@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_CONVEYOR_BELT_HPP
#define HEADER_SUPERTUX_OBJECT_CONVEYOR_BELT_HPP

#include "object/moving_sprite.hpp"

#include "supertux/timer.hpp"

/**
 * This class represents a platform that moves entities riding it.

 * @scripting
 * @summary A ""ConveyorBelt"" that was given a name can be controlled by scripts.
 * @instances A ""ConveyorBelt"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class ConveyorBelt final : public MovingSprite
{
public:
  static void register_class(ssq::VM& vm);

public:
  ConveyorBelt(const ReaderMapping& reader);

  ObjectSettings get_settings() override;

  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  static std::string class_name() { return "conveyor-belt"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "ConveyorBelt"; }
  static std::string display_name() { return _("Conveyor Belt"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual int get_layer() const override { return LAYER_TILES; }

  virtual void after_editor_set() override;

  /** @name Scriptable Methods */
  /**
   * @scripting
   * Starts the conveyor belt.
   */
  void start();
  /**
   * @scripting
   * Stops the conveyor belt.
   */
  void stop();
  /**
   * @scripting
   * Makes the conveyor shift objects to the left.
   */
  void move_left();
  /**
   * @scripting
   * Makes the conveyor shift objects to the right.
   */
  void move_right();
  /**
   * @scripting
   * Change the shifting speed of the conveyor.
   * @param float $target_speed
   */
  void set_speed(float target_speed);

private:
  void update_hitbox() override;

private:
  bool m_running;
  Direction m_dir;
  int m_length;
  float m_speed;

  float m_frame;
  int m_frame_index;

  const float MAX_SPEED = 32.0f;

private:
  ConveyorBelt(const ConveyorBelt&) = delete;
  ConveyorBelt& operator=(const ConveyorBelt&) = delete;
};


#endif

/* EOF */
