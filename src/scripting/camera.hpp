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
  void reload_config();

  /** Shake the camera */
  void shake(float speed, float x, float y);
  /** Set camera to a specific coordinate */
  void set_pos(float x, float y);
  /** Move the camera x to the left and y down */
  void move(float x, float y);
  /** Set camera to a specific mode, can be "normal", "manual" */
  void set_mode(const std::string& mode);
  /** Scroll camera to position x,y in scrolltime seconds */
  void scroll_to(float x, float y, float scrolltime);
  /** Get the curent scale factor of the camera */
  float get_current_scale();
  /** Get the scale factor the camera is fading towards */
  float get_target_scale();
  /** Set the scale factor */
  void set_scale(float scale);
  /** Fade the scale factor over time */
  void scale(float scale, float time);
  /** Fade the scale factor over time with easing (smooth movement) */
  void ease_scale(float scale, float time, const std::string& ease);
  /** Gets the current width of the screen */
  int get_screen_width();
  /** Gets the current height of the screen */
  int get_screen_height();
  /** Gets the x cooridnate of the top left corner of the screen */
  float get_x();
  /** Gets the y cooridnate of the top left corner of the screen */
  float get_y();
  
};

} // namespace scripting

#endif

/* EOF */
