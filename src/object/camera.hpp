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

#ifndef HEADER_SUPERTUX_OBJECT_CAMERA_HPP
#define HEADER_SUPERTUX_OBJECT_CAMERA_HPP

#include <memory>
#include <string>

#include "math/vector.hpp"
#include "object/path_object.hpp"
#include "scripting/camera.hpp"
#include "scripting/exposed_object.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

class Sector;
class Path;
class PathWalker;
class ReaderMapping;
class CameraConfig;

class Camera : public GameObject,
               public ExposedObject<Camera, scripting::Camera>,
               public PathObject {
 public:
  Camera(Sector* sector, const std::string& name = std::string());
  virtual ~Camera();
  virtual void save(Writer& writer);

  /// parse camera mode from lisp file
  void parse(const ReaderMapping& reader);

  /// reset camera position
  void reset(const Vector& tuxpos);

  /** return camera position */
  const Vector& get_translation() const;

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext&);

  // shake camera in a direction 1 time
  void shake(float speed, float x, float y);

  void set_scrolling(int scroll_x, int scroll_y)
  {
    translation.x = scroll_x;
    translation.y = scroll_y;
  }

  /**
   * scroll the upper left edge of the camera in scrolltime seconds
   * to the position goal
   */
  void scroll_to(const Vector& goal, float scrolltime);
  void move(const int dx, const int dy);

  void reload_config();

  enum CameraMode { NORMAL, MANUAL, AUTOSCROLL, SCROLLTO };
  CameraMode mode;

  /**
   * get the coordinates of the point directly in the center of this camera
   */
  Vector get_center() const;
  virtual bool is_saveable() const;
  std::string get_class() const { return "camera"; }
  std::string get_display_name() const { return _("Camera"); }

  virtual ObjectSettings get_settings();
  virtual void after_editor_set();

  virtual const std::string get_icon_path() const
  {
    return "images/engine/editor/camera.png";
  }

 private:
  void update_scroll_normal(float elapsed_time);
  void update_scroll_autoscroll(float elapsed_time);
  void update_scroll_to(float elapsed_time);
  void keep_in_bounds(Vector& vector);
  void shake();

 private:
  /**
   * The camera basically provides lookahead on the left or right side
   * or is undecided.
   */
  enum LookaheadMode { LOOKAHEAD_NONE, LOOKAHEAD_LEFT, LOOKAHEAD_RIGHT };

 private:
  Vector translation;

  Sector* sector;

  // normal mode
  LookaheadMode lookahead_mode;
  float changetime;
  Vector lookahead_pos;
  Vector peek_pos;
  Vector cached_translation;

  // shaking
  Timer shaketimer;
  float shakespeed;
  float shakedepth_x;
  float shakedepth_y;

  // scrollto mode
  Vector scroll_from;
  Vector scroll_goal;
  float scroll_to_pos;
  float scrollspeed;

  std::unique_ptr<CameraConfig> config;

 private:
  Camera(const Camera&);
  Camera& operator=(const Camera&);

  CameraMode defaultmode;
};

#endif /*SUPERTUX_CAMERA_H*/

/* EOF */
