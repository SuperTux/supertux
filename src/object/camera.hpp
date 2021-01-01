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

#include "math/size.hpp"
#include "math/vector.hpp"
#include "object/path_object.hpp"
#include "scripting/camera.hpp"
#include "squirrel/exposed_object.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

class Path;
class PathWalker;
class ReaderMapping;
class CameraConfig;

class Camera final : public GameObject,
                     public ExposedObject<Camera, scripting::Camera>,
                     public PathObject
{
public:
  enum class Mode
  {
    NORMAL, MANUAL, AUTOSCROLL, SCROLLTO
  };

private:
  /** The camera basically provides lookahead on the left or right
      side or is undecided. */
  enum class LookaheadMode {
    NONE, LEFT, RIGHT
  };

public:
  Camera(const std::string& name);
  Camera(const ReaderMapping& reader);
  ~Camera() override;

  /** \addtogroup GameObject
      @{ */
  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& ) override;

  virtual bool is_singleton() const override { return true; }
  virtual bool is_saveable() const override;

  virtual std::string get_class() const override { return "camera"; }
  virtual std::string get_display_name() const override { return _("Camera"); }

  virtual void backup(Writer& writer) const override;
  virtual void restore(const ReaderMapping& reader) override;

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual const std::string get_icon_path() const override { return "images/engine/editor/camera.png"; }
  /** @} */

  /** \addtogroup CameraAPI
      @{ */

  /** reset camera position */
  void reset(const Vector& tuxpos);

  /** return camera position */
  const Vector get_translation() const;
  void set_translation(const Vector& translation) { m_translation = translation; }

  /** shake camera in a direction 1 time */
  void shake(float duration, float x, float y);

  /** scroll the upper left edge of the camera in scrolltime seconds
      to the position goal */
  void scroll_to(const Vector& goal, float scrolltime);
  void move(const int dx, const int dy);

  void reload_config();

  /** get the coordinates of the point directly in the center of this
      camera */
  Vector get_center() const;

  void set_mode(Mode mode_) { m_mode = mode_; }

  /** get the exact scale at this exact moment */
  float get_current_scale() const { return m_scale; }

  /** get the scale towards which the camera is moving */
  float get_target_scale() const { return m_scale_target; }

  /** smoothly slide the scale of the camera towards a new value */
  void ease_scale(float scale, float time, easing ease);
  /** @} */

private:
  void update_scroll_normal(float dt_sec);
  void update_scroll_autoscroll(float dt_sec);
  void update_scroll_to(float dt_sec);
  void update_scale(float dt_sec);
  void keep_in_bounds(Vector& vector);
  void shake();

private:
  Mode m_mode;
  Mode m_defaultmode;

  Size m_screen_size;

  Vector m_translation;

  // normal mode
  LookaheadMode m_lookahead_mode;
  float m_changetime;
  Vector m_lookahead_pos;
  Vector m_peek_pos;
  Vector m_cached_translation;

  // shaking
  Timer m_shaketimer;
  float m_shakespeed;
  float m_shakedepth_x;
  float m_shakedepth_y;

  // scrollto mode
  Vector m_scroll_from;
  Vector m_scroll_goal;
  float m_scroll_to_pos;
  float m_scrollspeed;

  std::unique_ptr<CameraConfig> m_config;

  float m_scale,
        m_scale_origin,
        m_scale_target,
        m_scale_time_total,
        m_scale_time_remaining;
  easing m_scale_easing;

private:
  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;
};

#endif

/* EOF */
