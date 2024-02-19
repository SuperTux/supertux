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

#ifndef HEADER_SUPERTUX_SCRIPTING_BACKGROUND_HPP
#define HEADER_SUPERTUX_SCRIPTING_BACKGROUND_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Background;
#endif

namespace scripting {

/**
 * @summary A ""Background"" that was given a name can be manipulated by scripts.
 * @instances A ""Background"" can be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Background final
#ifndef SCRIPTING_API
  : public GameObject<::Background>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Background(const Background&) = delete;
  Background& operator=(const Background&) = delete;
#endif

public:
  /**
   * Sets the background's image.
   * @param string $image
   */
  void set_image(const std::string& image);
  /**
   * Sets the top, middle and bottom background images.
   * @param string $top_image
   * @param string $middle_image
   * @param string $bottom_image
   */
  void set_images(const std::string& top_image, const std::string& middle_image,
                             const std::string& bottom_image);
  /**
   * Sets the background speed.
   * @param float $speed
   */
  void set_speed(float speed);

  /**
   * Returns the red color value.
   */
  float get_color_red();
  /**
   * Returns the green color value.
   */
  float get_color_green();
  /**
   * Returns the blue color value.
   */
  float get_color_blue();
  /**
   * Returns the alpha color value.
   */
  float get_color_alpha();
  /**
   * Sets the background color.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   */
  void set_color(float red, float green, float blue, float alpha);
  /**
   * Fades to specified background color in ""time"" seconds.
   * @param float $red
   * @param float $green
   * @param float $blue
   * @param float $alpha
   * @param float $time
   */
  void fade_color(float red, float green, float blue, float alpha, float time);

  /**
   * Sets the sprite action for the top image.
   * @param string $action
   */
  void set_top_image_action(const std::string& action);
  /**
   * Sets the sprite action for the main (middle) image.
   * @param string $action
   */
  void set_image_action(const std::string& action);
  /**
   * Sets the sprite action for the bottom image.
   * @param string $action
   */
  void set_bottom_image_action(const std::string& action);
  /**
   * Sets the sprite action for all images (top, middle and bottom).
   * @param string $action
   */
  void set_image_actions(const std::string& action);
};

} // namespace scripting

#endif

/* EOF */
