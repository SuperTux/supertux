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
#include <string>
#include "scripting/game_object.hpp"

class ScriptedObject;
#endif

namespace scripting {

/**
 * @summary A ""ScriptedObject"" that was given a name can be controlled by scripts.
 * @instances A ""ScriptedObject"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class ScriptedObject final
#ifndef SCRIPTING_API
  : public GameObject<::ScriptedObject>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  ScriptedObject(const ScriptedObject&) = delete;
  ScriptedObject& operator=(const ScriptedObject&) = delete;
#endif

public:
  /**
   * Sets the sprite action/animation.
   * @param string $animation
   */
  void set_action(const std::string& animation);
  /**
   * Returns the current sprite action.
   */
  std::string get_action() const;

  /**
   * Returns the name of the object.
   */
  std::string get_name() const;

  /**
   * Moves the object by ""x"" units to the right and ""y"" down, relative to its current position.
   * @param float $x
   * @param float $y
   */
  void move(float x, float y);
  /**
   * Identical to ""move()"", except it's relative to the sector origin.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * Returns the X coordinate of the object's position.
   */
  float get_pos_x() const;
  /**
   * Returns the Y coordinate of the object's position.
   */
  float get_pos_y() const;

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
   * Shows or hides the object, according to the value of ""visible"".
   * @param bool $visible
   */
  void set_visible(bool visible);
  /**
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
};

} // namespace scripting

#endif

/* EOF */
