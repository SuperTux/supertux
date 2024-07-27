//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2023 Vankata453
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

#include <string>

#include "math/anchor_point.hpp"
#include "math/size.hpp"
#include "math/vector.hpp"
#include "object/path_object.hpp"
#include "supertux/game_object.hpp"
#include "supertux/timer.hpp"

class Path;
class PathWalker;
class ReaderMapping;

/**
 * @scripting
 * @summary A ""Camera"" that was given a name can be manipulated by scripts.
 * @instances An instance named ""Camera"" (""sector.Camera"" in the console) is available.${SRG_NEWPARAGRAPH}
              The mode of the camera is either ""normal"" (the camera is following the player) or ""autoscroll"". In the latter mode, the camera is forced along a specified path.
 */
class Camera final : public GameObject,
                     public PathObject
{
public:
  static void register_class(ssq::VM& vm);

public:
  enum class Mode
  {
    NORMAL, MANUAL, AUTOSCROLL, SCROLLTO
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

  static std::string class_name() { return "camera"; }
  virtual std::string get_class_name() const override { return class_name(); }
  virtual std::string get_exposed_class_name() const override { return "Camera"; }
  static std::string display_name() { return _("Camera"); }
  virtual std::string get_display_name() const override { return display_name(); }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  void save_state() override;
  void check_state() override;

  virtual const std::string get_icon_path() const override { return "images/engine/editor/camera.png"; }
  /** @} */

  Rectf get_rect() const;

  /** \addtogroup CameraAPI
      @{ */

  /** reset camera position */
  void reset(const Vector& tuxpos);

  /** return camera position */
  const Vector get_translation() const;
  void set_translation(const Vector& translation) { m_translation = translation; }
  void set_translation_centered(const Vector& translation);

  void keep_in_bounds(const Rectf& bounds);

  /** shake camera in a direction 1 time */
  void shake(float duration, float x, float y);

  /** Shake the camera vertically with a specified average strength, at a certain minimal delay, until stopped. */
  void start_earthquake(float strength, float delay);
  void stop_earthquake();

  /** scroll the upper left edge of the camera in scrolltime seconds
      to the position goal */
  void scroll_to(const Vector& goal, float scrolltime);
  void move(const Vector& offset);

  /** get the coordinates of the point directly in the center of this
      camera */
  Vector get_center() const;

  /** get the coordinates of the point directly in the top left of this
      camera */
  const Vector& get_position() const;

  /** get the width and height of the screen*/
  const Sizef& get_screen_size() const;

  void set_mode(Mode mode_) { m_mode = mode_; }

  Mode get_mode() const { return m_mode; }

  /** get the exact scale at this exact moment */
  float get_current_scale() const { return m_enfore_minimum_scale ? std::min(m_minimum_scale, m_scale) : m_scale; }

  /** get the scale towards which the camera is moving */
  float get_target_scale() const { return m_scale_target; }

  /** smoothly slide the scale and anchor position of the camera towards a new value */
  void ease_scale(float scale, float time, easing ease, AnchorPoint anchor = AnchorPoint::ANCHOR_MIDDLE);
  /** @} */

  /**
   * @scripting
   * @description Moves the camera to the specified absolute position. The origin is at the top left.
   * @param float $x
   * @param float $y
   */
  void set_pos(float x, float y);
  /**
   * @scripting
   * @description Moves the camera ""x"" to the left and ""y"" down.
   * @param float $x
   * @param float $y
   */
  void move(float x, float y);
  /**
   * @scripting
   * @description Sets the camera mode.
   * @param string $mode The mode can be "normal" or "manual".
   */
  void set_mode(const std::string& mode);
  /**
   * @scripting
   * @description Scrolls the camera to specific coordinates in ""scrolltime"" seconds.
   * @param float $x
   * @param float $y
   * @param float $scrolltime
   */
  void scroll_to(float x, float y, float scrolltime);
  /**
   * @scripting
   * @description Sets the scale factor.
   * @param float $scale
   */
  void set_scale(float scale);
  /**
   * @scripting
   * @description Sets the scale factor and the target position anchor.
                  NOTE: Target position anchor is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param int $anchor Anchor point as represented by the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   */
  void set_scale_anchor(float scale, int anchor);
  /**
   * @scripting
   * @description Fades to a specified scale factor in ""time"" seconds.
   * @param float $scale
   * @param float $time
   */
  void scale(float scale, float time);
  /**
   * @scripting
   * @description Fades to a specified scale factor and target position anchor in ""time"" seconds.
                  NOTE: Target position anchor is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param float $time
   * @param int $anchor Anchor point as represented by the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   */
  void scale_anchor(float scale, float time, int anchor);
  /**
   * @scripting
   * @description Fades to a specified scale factor in ""time"" seconds with easing (smooth movement).
   * @param float $scale
   * @param float $time
   * @param string $ease
   */
  void ease_scale(float scale, float time, const std::string& ease);
  /**
   * @scripting
   * @description Fades to a specified scale factor and target position anchor in ""time"" seconds with easing (smooth movement).
                  NOTE: Target position anchor is only applied, if the camera is in "manual" mode.
   * @param float $scale
   * @param float $time
   * @param int $anchor Anchor point as represented by the ""ANCHOR_*"" constants (see ${SRG_REF_AnchorPoints}).
   * @param string $ease
   */
  void ease_scale_anchor(float scale, float time, int anchor, const std::string& ease);
  /**
   * @scripting
   * @description Gets the current width of the screen.
   */
  float get_screen_width() const;
  /**
   * @scripting
   * @description Gets the current height of the screen.
   */
  float get_screen_height() const;
  /**
   * @scripting
   * @description Gets the X coordinate of the top-left corner of the screen.
   */
  float get_x() const;
  /**
   * @scripting
   * @description Gets the Y coordinate of the top-left corner of the screen.
   */
  float get_y() const;

private:
  void keep_in_bounds(Vector& vector);

  void update_scroll_normal(float dt_sec);
  void update_scroll_normal_multiplayer(float dt_sec);
  void update_scroll_autoscroll(float dt_sec);
  void update_scroll_to(float dt_sec);
  void update_scale(float dt_sec);
  void update_shake();
  void update_earthquake();

  Vector get_scale_anchor_target() const;
  void reload_scale();

private:
  Mode m_mode;
  Mode m_defaultmode;

  Sizef m_screen_size;

  Vector m_translation;

  // normal mode
  Vector m_lookahead_pos;
  Vector m_peek_pos;
  Vector m_cached_translation;

  // shaking
  Timer m_shaketimer;
  float m_shakespeed;
  float m_shakedepth_x;
  float m_shakedepth_y;

  // Earthquake
  bool m_earthquake;
  float m_earthquake_strength,
        m_earthquake_delay,
        m_earthquake_last_offset;
  Timer m_earthquake_delay_timer;

  // scrollto mode
  Vector m_scroll_from;
  Vector m_scroll_goal;
  float m_scroll_to_pos;
  float m_scrollspeed;

  float m_scale,
        m_scale_origin,
        m_scale_target,
        m_scale_time_total,
        m_scale_time_remaining;
  Vector m_scale_origin_translation,
         m_scale_target_translation;
  easing m_scale_easing;
  AnchorPoint m_scale_anchor;

  // Minimum scale is used in certain circumstances where a fixed minimum scale
  // should be used, regardless of the scriping-accessible `m_scale` property.
  float m_minimum_scale;
  bool m_enfore_minimum_scale;

private:
  Camera(const Camera&) = delete;
  Camera& operator=(const Camera&) = delete;
};

#endif

/* EOF */
