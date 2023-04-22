//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SPOTLIGHT_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Spotlight;
#endif

namespace scripting {

/**
 * @summary A ""Spotlight"" that was given a name can be controlled by scripts.
 * @instances A ""Spotlight"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Spotlight final
#ifndef SCRIPTING_API
  : public GameObject<::Spotlight>
#endif
{
#ifndef SCRIPTING_API
private:
  using GameObject::GameObject;

private:
  Spotlight(const Spotlight&) = delete;
  Spotlight& operator=(const Spotlight&) = delete;
#endif

public:
  /**
   * Enables/disables the spotlight.
   * @param bool $enabled
   */
  void set_enabled(bool enabled);
  /**
   * Returns ""true"" if the spotlight is enabled.
   */
  bool is_enabled();

  /**
   * Sets the direction of the spotlight.
   * @param string $direction
   */
  void set_direction(const std::string& direction);

  /**
   * Sets the angle of the spotlight.
   * @param float $angle
   */
  void set_angle(float angle);
  /**
   * Fades the angle of the spotlight in ""time"" seconds.
   * @param float $angle
   * @param float $time
   */
  void fade_angle(float angle, float time);
  /**
   * Fades the angle of the spotlight in ""time"" seconds, with easing.
   * @param float $angle
   * @param float $time
   * @param string $easing
   */
  void ease_angle(float angle, float time, const std::string& easing);

  /**
   * Sets the speed of the spotlight.
   * @param float $speed
   */
  void set_speed(float speed);
  /**
   * Fades the speed of the spotlight in ""time"" seconds.
   * @param float $speed
   * @param float $time
   */
  void fade_speed(float speed, float time);
  /**
   * Fades the speed of the spotlight in ""time"" seconds, with easing.
   * @param float $speed
   * @param float $time
   * @param string $easing
   */
  void ease_speed(float speed, float time, const std::string& easing);

  /**
   * Sets the RGBA color of the spotlight.
   * @param float $r
   * @param float $g
   * @param float $b
   * @param float $a
   */
  void set_color_rgba(float r, float g, float b, float a);
  /**
   * Fades the spotlight to a new RGBA color in ""time"" seconds.
   * @param float $r
   * @param float $g
   * @param float $b
   * @param float $a
   * @param float $time
   */
  void fade_color_rgba(float r, float g, float b, float a, float time);
  /**
   * Fades the spotlight to a new RGBA color in ""time"" seconds, with easing.
   * @param float $r
   * @param float $g
   * @param float $b
   * @param float $a
   * @param float $time
   * @param string $easing
   */
  void ease_color_rgba(float r, float g, float b, float a, float time, std::string easing);
};

} // namespace scripting

#endif

/* EOF */
