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

#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "supertux/script_interface.hpp"
#include "supertux/timer.hpp"
#include "util/reader_fwd.hpp"

class Sector;
class Path;
class PathWalker;
class CameraConfig;

class Camera : public GameObject,
               public ScriptInterface
{
public:
  Camera(Sector* sector, std::string name = "");
  virtual ~Camera();
  virtual void save(lisp::Writer& writer);

  /// parse camera mode from lisp file
  void parse(const ReaderMapping& reader);

  /// reset camera position
  void reset(const Vector& tuxpos);

  /** return camera position */
  const Vector& get_translation() const;

  virtual void update(float elapsed_time);

  virtual void draw(DrawingContext& );

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx);
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx);

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

  enum CameraMode
  {
    NORMAL, AUTOSCROLL, MANUAL, SCROLLTO
  };
  CameraMode mode;

  /**
   * get the coordinates of the point directly in the center of this camera
   */
  Vector get_center() const;
  virtual std::string get_class() {
    return "camera";
  }

  virtual ObjectSettings get_settings();

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
  enum LookaheadMode {
    LOOKAHEAD_NONE, LOOKAHEAD_LEFT, LOOKAHEAD_RIGHT
  };

private:
  Vector translation;

  Sector* sector;

  // normal mode
  LookaheadMode lookahead_mode;
  float changetime;
  Vector lookahead_pos;
  Vector peek_pos;
  Vector cached_translation;

  // autoscroll mode
  std::unique_ptr<Path> autoscroll_path;
  std::unique_ptr<PathWalker> autoscroll_walker;

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
