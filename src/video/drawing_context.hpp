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
#include <boost/optional.hpp>

#include "math/rect.hpp"
#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/canvas.hpp"
#include "video/font.hpp"
#include "video/font_ptr.hpp"

struct DrawingRequest;
class VideoSystem;

class Transform
{
public:
  Vector translation;
  DrawingEffect drawing_effect;
  float alpha;

  Transform() :
    translation(),
    drawing_effect(NO_EFFECT),
    alpha(1.0f)
  { }

  Vector apply(const Vector& v) const
  {
    return v - translation;
  }
};

class CanvasGroup
{
private:
  boost::optional<Rect> m_clip_rect;
  Canvas m_colormap_canvas;
  Canvas m_lightmap_canvas;

public:
  CanvasGroup(DrawingContext& context, obstack& obst) :
    m_clip_rect(),
    m_colormap_canvas(NORMAL, context, obst),
    m_lightmap_canvas(LIGHTMAP, context, obst)
  {}

  ~CanvasGroup()
  {
    m_colormap_canvas.clear();
    m_lightmap_canvas.clear();
  }

  void set_clip_rect(const Rect& clip_rect)
  {
    m_clip_rect = clip_rect;
  }

  Rect get_clip_rect() const
  {
    return *m_clip_rect;
  }

  bool has_clip_rect() const
  {
    return static_cast<bool>(m_clip_rect);
  }

  Canvas& color() { return m_colormap_canvas; }
  Canvas& light() { return m_lightmap_canvas; }

  void clear()
  {
    m_lightmap_canvas.clear();
    m_colormap_canvas.clear();
  }
};

/**
 * This class provides functions for drawing things on screen. It also
 * maintains a stack of transforms that are applied to graphics.
 */
class DrawingContext
{
public:
  static bool render_lighting;

  DrawingContext(VideoSystem& video_system);
  ~DrawingContext();

  /// Returns the visible area in world coordinates
  Rectf get_cliprect() const;

  /// Processes all pending drawing requests and flushes the list.
  void do_drawing();

  /// on next update, set color to lightmap's color at position
  void get_light(const Vector& position, Color* color_out);

  typedef ::Target Target;
  static const Target NORMAL = ::NORMAL;
  static const Target LIGHTMAP = ::LIGHTMAP;

  Canvas& color() { return current_canvas_group().color(); }
  Canvas& light() { return current_canvas_group().light(); }
  Canvas& get_canvas(Target target) {
    switch(target)
    {
      case LIGHTMAP:
        return current_canvas_group().light();

      default:
        return current_canvas_group().color();
    }
  }

  void set_ambient_color(Color ambient_color);

  void push_transform();
  void pop_transform();
  const Transform& get_transform() const { return m_transform; }

  const Vector& get_translation() const
  {  return m_transform.translation;  }

  void set_translation(const Vector& newtranslation)
  {  m_transform.translation = newtranslation;  }

  /// Apply that effect in the next draws (effects are listed on surface.h).
  void set_drawing_effect(DrawingEffect effect);
  /// return currently applied drawing effect
  DrawingEffect get_drawing_effect() const;

  /// apply that alpha in the next draws (1.0 means fully opaque) */
  void set_alpha(float alpha);
  /// return currently set alpha
  float get_alpha() const;

private:
  CanvasGroup& current_canvas_group() { return *m_canvas_groups.back(); }

private:
  VideoSystem& m_video_system;

  /* obstack holding the memory of the drawing requests */
  struct obstack m_obst;

  std::vector<std::unique_ptr<CanvasGroup> > m_canvas_groups;

  Color m_ambient_color;

  /// the transform stack
  std::vector<Transform> m_transformstack;
  /// the currently active transform
  Transform m_transform;

private:
  DrawingContext(const DrawingContext&);
  DrawingContext& operator=(const DrawingContext&);
};

#endif

/* EOF */
