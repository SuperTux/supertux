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

class Sprite
{
public:
  Sprite(SpriteData& data);

  SpritePtr clone() const;

  /** Draw sprite, automatically calculates next frame */
  void draw(Canvas& canvas, const Vector& pos, int layer,
            DrawingEffect effect = NO_EFFECT);

  void draw_part(Canvas& canvas, const Vector& source,
                 const Vector& size, const Vector& pos, int layer);

  /** Set action (or state) */
  void set_action(const std::string& name, int loops = -1);

  /** Set action (or state), but keep current frame number, loop counter, etc. */
  void set_action_continued(const std::string& name);

  /** Set number of animation cycles until animation stops */
  void set_animation_loops(int loops = -1)
  { animation_loops = loops; }

  /* Stop animation */
  void stop_animation()
  { animation_loops = 0; }
  /** Check if animation is stopped or not */
  bool animation_done() const;

  float get_fps() const
  { return action->fps; }
  /** Get current action total frames */
  size_t get_frames() const
  { return action->surfaces.size(); }
  /** Get sprite's name */
  const std::string& get_name() const
  { return data.name; }
  /** Get current action name */
  const std::string& get_action() const
  { return action->name; }

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

  void set_blend(const Blend& blend);

  Blend get_blend() const;

  /** Get current frame */
  unsigned int get_frame() const
  { return frameidx; }
  /** Set current frame */
  void set_frame(unsigned int frame_)
  {
    frame = 0;
    frameidx = frame_ % get_frames();
  }
  SurfacePtr get_frame(unsigned int frame_) const
  {
    assert(frame_ < action->surfaces.size());
    return action->surfaces[frame_];
  }

  bool has_action (const std::string& name) const
  {
    return (data.get_action(name) != NULL);
  }

private:
  void update();

  SpriteData& data;

  // between 0 and 1
  float frame;
  // between 0 and get_frames()
  unsigned int frameidx;
  int   animation_loops;
  float last_ticks;
  float angle;
  Color color;
  Blend blend;

  const SpriteData::Action* action;

private:
  Sprite(const Sprite& other);
  Sprite& operator=(const Sprite&);
};

#endif

/* EOF */
