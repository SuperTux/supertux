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

#ifndef HEADER_SUPERTUX_VIDEO_DRAWING_CONTEXT_HPP
#define HEADER_SUPERTUX_VIDEO_DRAWING_CONTEXT_HPP

#include <string>
#include <vector>
#include <obstack.h>
#include <optional>

#include "math/rect.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/canvas.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/drawing_transform.hpp"
#include "video/font.hpp"
#include "video/font_ptr.hpp"

class VideoSystem;
struct DrawingRequest;
struct obstack;

/** This class provides functions for drawing things on screen. It
    also maintains a stack of transforms that are applied to
    graphics. */
class DrawingContext final
{
public:
  DrawingContext(VideoSystem& video_system, obstack& obst, bool overlay, float time_offset);
  ~DrawingContext();

  /** Returns the visible area in world coordinates */
  Rectf get_cliprect() const;

  Canvas& color() { return m_colormap_canvas; }
  Canvas& light() { assert(!m_overlay); return m_lightmap_canvas; }
  Canvas& get_canvas(DrawingTarget target) {
    switch (target)
    {
      case DrawingTarget::LIGHTMAP:
        return light();

      default:
        return color();
    }
  }

  void set_ambient_color(Color ambient_color);
  Color get_ambient_color() const { return m_ambient_color; }

  void push_transform();
  void pop_transform();
  DrawingTransform& transform();
  const DrawingTransform& transform() const;

  const Vector& get_translation() const
  {  return transform().translation;  }

  void set_translation(const Vector& newtranslation)
  {  transform().translation = newtranslation;  }

  float get_scale() const { return transform().scale; }
  void scale(float scale) { transform().scale *= scale; }

  /** Apply that flip in the next draws (flips are listed on surface.h). */
  void set_flip(Flip flip);
  Flip get_flip() const;

  /** apply that alpha in the next draws (1.0 means fully opaque) */
  void set_alpha(float alpha);
  float get_alpha() const;

  /** For position extrapolation at high frame rates: real time since last game update step */
  void set_time_offset(float time_offset) { m_time_offset = time_offset; }
  float get_time_offset() const { return m_time_offset; }

  void clear()
  {
    m_lightmap_canvas.clear();
    m_colormap_canvas.clear();
  }

  void set_viewport(const Rect& viewport)
  {
    transform().viewport = viewport;
  }

  const Rect& get_viewport() const;

  float get_width() const;
  float get_height() const;
  Vector get_size() const;
  Rectf get_rect() const { return Rectf(Vector(0, 0), get_size()); }

  bool use_lightmap() const
  {
    return !m_overlay && m_ambient_color != Color::WHITE;
  }

  bool is_overlay() const { return m_overlay; }

private:
  VideoSystem& m_video_system;

  /** obstack holds the memory of all the drawing requests, it is
      shared with the Canvas */
  obstack& m_obst;

  /** A context marked as overlay will not have it's light section
      rendered. */
  bool m_overlay;

  Color m_ambient_color;
  std::vector<DrawingTransform> m_transform_stack;

  Canvas m_colormap_canvas;
  Canvas m_lightmap_canvas;

  float m_time_offset;

private:
  DrawingContext(const DrawingContext&) = delete;
  DrawingContext& operator=(const DrawingContext&) = delete;
};

#endif

/* EOF */
