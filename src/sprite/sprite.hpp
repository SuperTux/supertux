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

#ifndef HEADER_SUPERTUX_SPRITE_SPRITE_HPP
#define HEADER_SUPERTUX_SPRITE_SPRITE_HPP

#include "sprite/sprite_data.hpp"
#include "sprite/sprite_ptr.hpp"
#include "video/canvas.hpp"
#include "video/drawing_context.hpp"

class Sprite final
{
public:
  Sprite(SpriteData& data);
  ~Sprite();

  SpritePtr clone() const;

  /** Draw sprite, automatically calculates next frame */
  void draw(Canvas& canvas, const Vector& pos, int layer,
            Flip flip = NO_FLIP);

  /** Set action (or state) */
  void set_action(const std::string& name, int loops = -1);

  /** Set action (or state), but keep current frame number, loop counter, etc. */
  void set_action_continued(const std::string& name);

  /** Set number of animation cycles until animation stops */
  void set_animation_loops(int loops = -1) { m_animation_loops = loops; }

  void set_frame_progress(float frame_progress) { m_frame = frame_progress; }

  void set_frame(int frame) { m_frameidx = frame; }

  /* Stop animation */
  void stop_animation() { m_animation_loops = 0; }

  /** Check if animation is stopped or not */
  bool animation_done() const;

  /** Get current action total frames */
  int get_frames() const { return static_cast<int>(m_action->surfaces.size()); }

  /** Get currently drawn frame */
  int get_current_frame() const { return m_frameidx; }

  /** Get current frame progress */
  float get_current_frame_progress() const { return m_frame; }

  /** Get sprite's name */
  const std::string& get_name() const { return m_data.name; }

  /** Get current action name */
  const std::string& get_action() const { return m_action->name; }

  int get_width() const;
  int get_height() const;

  /** return x-offset of current action's hitbox, relative to start of image */
  float get_current_hitbox_x_offset() const;
  /** return y-offset of current action's hitbox, relative to start of image */
  float get_current_hitbox_y_offset() const;
  /** return width of current action's hitbox */
  float get_current_hitbox_width() const;
  /** return height of current action's hitbox */
  float get_current_hitbox_height() const;
  /** return current action's hitbox, relative to 0,0 */
  Rectf get_current_hitbox() const;

  /** Set the angle of the sprite rotation in degree */
  void set_angle(float angle);

  /** Get the angle of the sprite rotation in degree */
  float get_angle() const;

  void set_color(const Color& color);
  Color get_color() const;

  void set_alpha(float alpha);
  float get_alpha() const;

  void set_blend(const Blend& blend);
  Blend get_blend() const;

  bool has_action (const std::string& name) const { return (m_data.get_action(name) != nullptr); }

private:
  void update();

  SpriteData& m_data;

  // between 0 and 1
  float m_frame;
  // between 0 and get_frames()
  int m_frameidx;
  int m_animation_loops;
  float m_last_ticks;
  float m_angle;
  float m_alpha;
  Color m_color;
  Blend m_blend;

  const SpriteData::Action* m_action;

private:
  Sprite(const Sprite& other);
  Sprite& operator=(const Sprite&) = delete;
};

#endif

/* EOF */
