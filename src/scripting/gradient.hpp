//  SuperTux - Sector scripting
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_GRADIENT_HPP
#define HEADER_SUPERTUX_SCRIPTING_GRADIENT_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Gradient;
#endif

namespace scripting {

/**
 * @summary A ""Gradient"" that was given a name can be controlled by scripts.
 * @instances A ""Gradient"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Gradient final
#ifndef SCRIPTING_API
  : public GameObject<::Gradient>
#endif
{
#ifndef SCRIPTING_API
private:
  using GameObject::GameObject;

private:
  Gradient(const Gradient&) = delete;
  Gradient& operator=(const Gradient&) = delete;
#endif

public:
  /**
   * Sets the direction of the gradient.
   * @param string $direction Can be "horizontal", "vertical", "horizontal_sector" or "vertical_sector".
   */
  void set_direction(const std::string& direction);
  /**
   * Returns the direction of the gradient.
     Possible values are "horizontal", "vertical", "horizontal_sector" or "vertical_sector".
   */
  std::string get_direction() const;

  /**
   * Set top gradient color.
   * @param float $red
   * @param float $green
   * @param float $blue
   */
  void set_color1(float red, float green, float blue);
  /**
   * Set bottom gradient color.
   * @param float $red
   * @param float $green
   * @param float $blue
   */
  void set_color2(float red, float green, float blue);
  /**
   * Set both gradient colors.
   * @param float $red1
   * @param float $green1
   * @param float $blue1
   * @param float $red2
   * @param float $green2
   * @param float $blue2
   */
  void set_colors(float red1, float green1, float blue1, float red2, float green2, float blue2);
  /**
   * Fade the top gradient color to a specified new color in ""time"" seconds.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $time
   */
  void fade_color1(float red, float green, float blue, float time);
  /**
   * Fade the bottom gradient color to a specified new color in ""time"" seconds.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $time
   */
  void fade_color2(float red, float green, float blue, float time);
  /**
   * Fade both gradient colors to specified new colors in ""time"" seconds.
   * @param float $red1
   * @param float $green1
   * @param float $blue1
   * @param float $red2
   * @param float $green2
   * @param float $blue2
   * @param float $time
   */
  void fade_colors(float red1, float green1, float blue1, float red2, float green2, float blue2, float time);

  /**
   * Swap top and bottom gradient colors.
   */
  void swap_colors();
};

} // namespace scripting

#endif

/* EOF */
