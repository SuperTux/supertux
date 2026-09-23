//  SuperTux
//  Copyright (C) 2026 Tobias Markus <tobbi.bugs@googlemail.com>
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

#pragma once

#include "math/vector.hpp"

class Camera;

/**
 * Class responsible for camera navigation / handling in the SuperTux editor
 */
class EditorCamera
{
public:

  /**
   * Constructor
   */
  EditorCamera();

  /**
   * Returns the scroll speed that is used for moving the camera
   * @return Speed of the camera
   */
  float get_scroll_speed() const { return m_scroll_speed; }

  /**
   * Sets the scroll speed that is used for moving the camera
   * @param scroll_speed The speed of the camera
   */
  void set_scroll_speed(float scroll_speed) { m_scroll_speed = scroll_speed; }

  /**
   * Returns the scale of the camera
   */
  float get_scale() const { return m_new_scale; }

  /**
   * Sets the scale of the camera
   * @param value Camera scale value
   */
  void set_scale(float value) { m_new_scale = value; }

  /**
   * Scrolls the camera by the specified velocity
   * @param velocity The scroll velocity as a vector with x / y coordinates
   */
  void scroll(const Vector& velocity);

  /**
   * Update function, handles applying new scale, among other things
   * @param camera The camera to update
   * @param dt_sec Elapsed time since last update
   */
  void update(Camera &camera, float dt_sec);

  /**
   * Makes sure that the camera does not exceed window boundaries
   */
  void keep_in_bounds();

private:
  float m_scroll_speed;
  float m_new_scale;
};
