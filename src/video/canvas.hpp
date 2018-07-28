//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_CANVAS_HPP
#define HEADER_SUPERTUX_VIDEO_CANVAS_HPP

#include <string>
#include <vector>
#include <obstack.h>

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/font.hpp"
#include "video/font_ptr.hpp"
#include "video/drawing_target.hpp"

struct DrawingRequest;
class VideoSystem;
class DrawingContext;

// some constants for predefined layer values
enum {
  // Image/gradient backgrounds (should cover entire screen)
  LAYER_BACKGROUND0 = -300,
  // Particle backgrounds
  LAYER_BACKGROUND1 = -200,
  // Tilemap backgrounds
  LAYER_BACKGROUNDTILES = -100,
  // Solid tilemaps
  LAYER_TILES = 0,
  // Ordinary objects
  LAYER_OBJECTS = 50,
  // Objects that pass through walls
  LAYER_FLOATINGOBJECTS = 150,
  //
  LAYER_FOREGROUNDTILES = 200,
  //
  LAYER_FOREGROUND0 = 300,
  //
  LAYER_FOREGROUND1 = 400,

  LAYER_LIGHTMAP = 450,

  // Hitpoints, time, coins, etc.
  LAYER_HUD = 500,
  // Menus, mouse, console etc.
  LAYER_GUI         = 600
};

enum GradientDirection { VERTICAL, HORIZONTAL, VERTICAL_SECTOR, HORIZONTAL_SECTOR };

class Blend
{
public:
  GLenum sfactor;
  GLenum dfactor;

  Blend()
    : sfactor(GL_SRC_ALPHA), dfactor(GL_ONE_MINUS_SRC_ALPHA)
  {}

  Blend(GLenum s, GLenum d)
    : sfactor(s), dfactor(d)
  {}
};

class Canvas
{
public:
  enum Filter { BELOW_LIGHTMAP, ABOVE_LIGHTMAP, ALL };

public:
  Canvas(DrawingTarget target, DrawingContext& context, obstack& obst);
  ~Canvas();

  void draw_surface(SurfacePtr surface, const Vector& position,
                    int layer);
  void draw_surface(SurfacePtr surface, const Vector& position,
                    float angle, const Color& color, const Blend& blend,
                    int layer);
  void draw_surface_part(SurfacePtr surface,
                         const Rectf& srcrect, const Rectf& dstrect,
                         int layer);
  void draw_text(FontPtr font, const std::string& text,
                 const Vector& position, FontAlignment alignment, int layer, Color color = Color(1.0,1.0,1.0));
  /** Draw text to the center of the screen */
  void draw_center_text(FontPtr font, const std::string& text,
                        const Vector& position, int layer, Color color = Color(1.0,1.0,1.0));
  void draw_gradient(const Color& from, const Color& to, int layer, const GradientDirection& direction, const Rectf& region);
  void draw_filled_rect(const Vector& topleft, const Vector& size,
                        const Color& color, int layer);
  void draw_filled_rect(const Rectf& rect, const Color& color, int layer);
  void draw_filled_rect(const Rectf& rect, const Color& color, float radius, int layer);

  void draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer);

  void draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer);
  void draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer);

  void clear();
  void render(VideoSystem& video_system, Filter filter);

  DrawingContext& get_context() { return m_context; }

  // FIXME: this function is temporary only, should be refactored out
  std::vector<DrawingRequest*>& get_requests() { return m_requests; }

private:
  Vector apply_translate(const Vector& pos) const;

private:
  DrawingTarget m_target;
  DrawingContext& m_context;
  obstack& m_obst;
  std::vector<DrawingRequest*> m_requests;

private:
  Canvas(const Canvas&) = delete;
  Canvas& operator=(const Canvas&) = delete;
};

#endif

/* EOF */
