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

#ifndef HEADER_SUPERTUX_SCRIPTING_CAMERA_HPP
#define HEADER_SUPERTUX_SCRIPTING_CAMERA_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Camera;
#endif

namespace scripting {

/**
 * @summary A ""Camera"" that was given a name can be manipulated by scripts.
 * @instances An instance named ""Camera"" (""sector.Camera"" in the console) is available.${SRG_NEWPARAGRAPH}
              The mode of the camera is either normal (the camera is following the player) or autoscroll. In the latter mode the camera is forced along a specified ${SRG_REF_Path}.
 */
class Camera final
#ifndef SCRIPTING_API
  : public GameObject<::Camera>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;
#endif

public:
  /**
   * Reloads the camera's configuration.
   */
  void reload_config();

  /**
   * Moves camera to the given coordinates in ""time"" seconds, returning quickly to the original position afterwards.
   * @param float $speed
   * @param float $x
   * @param float $y
   */
  void shake(float speed, float x, float y);
  /**
   * Moves the camera to the specified absolute position. The origin is at the top left.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * Moves the camera ""x"" to the left and ""y"" down.
   * @param float $x
   * @param float $y
   */
  void move(float x, float y);
  /**
   * Sets the camera mode.
   * @param string $mode The mode can be "normal" or "manual".
   */
  void set_mode(const std::string& mode);
  /**
   * Scrolls the camera to specific coordinates in ""scrolltime"" seconds.
   * @param float $x
   * @param float $y
   * @param float $scrolltime
   */
  void scroll_to(float x, float y, float scrolltime);
  /**
   * Returns the current scale factor of the camera.
   */
  float get_current_scale();
  /**
   * Returns the scale factor the camera is fading towards.
   */
  float get_target_scale();
  /**
   * Sets the scale factor.
     NOTE: If the camera is in "manual" mode, it is required to provide a target center position. See the function below.
   * @param float $scale
   */
  void set_scale(float scale);
  /**
   * Sets the scale factor and the target center position.
     NOTE: Target center position is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param float $centerX
   * @param float $centerY
   */
  void set_scale_target(float scale, float centerX, float centerY);
  /**
   * Fades to a specified scale factor in ""time"" seconds.
     NOTE: If the camera is in "manual" mode, it is required to provide a target center position. See the function below.
   * @param float $scale
   * @param float $time
   */
  void scale(float scale, float time);
  /**
   * Fades to a specified scale factor and target center position in ""time"" seconds.
     NOTE: Target center position is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param float $time
   * @param float $centerX
   * @param float $centerY
   */
  void scale_target(float scale, float time, float centerX, float centerY);
  /**
   * Fades to a specified scale factor in ""time"" seconds with easing (smooth movement).
     NOTE: If the camera is in "manual" mode, it is required to provide a target center position. See the function below.
   * @param float $scale
   * @param float $time
   * @param string $ease
   */
  void ease_scale(float scale, float time, const std::string& ease);
  /**
   * Fades to a specified scale factor and target center position in ""time"" seconds with easing (smooth movement).
     NOTE: Target center position is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param float $time
   * @param float $centerX
   * @param float $centerY
   * @param string $ease
   */
  void ease_scale_target(float scale, float time, float centerX, float centerY, const std::string& ease);
  /**
   * Gets the current width of the screen.
   */
  int get_screen_width();
  /**
   * Gets the current height of the screen.
   */
  int get_screen_height();
  /**
   * Gets the X coordinate of the top-left corner of the screen.
   */
  float get_x();
  /**
   * Gets the Y coordinate of the top-left corner of the screen.
   */
  float get_y();
};

} // namespace scripting

#endif

/* EOF */
