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

#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include <obstack.h>

#include "math/rectf.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/font.hpp"
#include "video/font_ptr.hpp"
#include "video/texture.hpp"

struct DrawingRequest;
class Surface;
class Texture;
class VideoSystem;

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

enum Target {
  NORMAL, LIGHTMAP
};

/**
 * This class provides functions for drawing things on screen. It also
 * maintains a stack of transforms that are applied to graphics.
 */
class DrawingContext
{
public:
  DrawingContext(VideoSystem& video_system);
  ~DrawingContext();

  /// Adds a drawing request for a surface into the request list.
  void draw_surface(SurfacePtr surface, const Vector& position,
                    int layer);
  /// Adds a drawing request for a surface into the request list.
  void draw_surface(SurfacePtr surface, const Vector& position,
                    float angle, const Color& color, const Blend& blend,
                    int layer);
  /// Adds a drawing request for part of a surface.
  void draw_surface_part(SurfacePtr surface,
                         const Rectf& srcrect, const Rectf& dstrect,
                         int layer);
  /// Draws a text.
  void draw_text(FontPtr font, const std::string& text,
                 const Vector& position, FontAlignment alignment, int layer, Color color = Color(1.0,1.0,1.0));

  /// Draws text on screen center (feed Vector.x with a 0).
  /// This is the same as draw_text() with a SCREEN_WIDTH/2 position and
  /// alignment set to LEFT_ALIGN
  void draw_center_text(FontPtr font, const std::string& text,
                        const Vector& position, int layer, Color color = Color(1.0,1.0,1.0));
  /// Draws a color gradient onto the whole screen */
  void draw_gradient(const Color& from, const Color& to, int layer, const GradientDirection& direction, const Rectf& region);
  /// Fills a rectangle.
  void draw_filled_rect(const Vector& topleft, const Vector& size,
                        const Color& color, int layer);
  void draw_filled_rect(const Rectf& rect, const Color& color, int layer);
  void draw_filled_rect(const Rectf& rect, const Color& color, float radius, int layer);

  void draw_inverse_ellipse(const Vector& pos, const Vector& size, const Color& color, int layer);

  void draw_line(const Vector& pos1, const Vector& pos2, const Color& color, int layer);
  void draw_triangle(const Vector& pos1, const Vector& pos2, const Vector& pos3, const Color& color, int layer);

  /// Returns the visible area in world coordinates
  Rectf get_cliprect() const;

  /// Processes all pending drawing requests and flushes the list.
  void do_drawing();

  const Vector& get_translation() const
  {  return transform.translation;  }

  void set_translation(const Vector& newtranslation)
  {  transform.translation = newtranslation;  }

  void push_transform();
  void pop_transform();

  /// Apply that effect in the next draws (effects are listed on surface.h).
  void set_drawing_effect(DrawingEffect effect);
  /// return currently applied drawing effect
  DrawingEffect get_drawing_effect() const;
  /// apply that alpha in the next draws (1.0 means fully opaque) */
  void set_alpha(float alpha);
  /// return currently set alpha
  float get_alpha() const;

  /// on next update, set color to lightmap's color at position
  void get_light(const Vector& position, Color* color );

  typedef ::Target Target;
  static const Target NORMAL = ::NORMAL;
  static const Target LIGHTMAP = ::LIGHTMAP;
  void push_target();
  void pop_target();
  void set_target(Target target);

  void set_ambient_color( Color new_color );

  /**
   * requests that a screenshot be taken after the next frame has been rendered
   */
  void take_screenshot();

private:
  typedef std::vector<DrawingRequest*> DrawingRequests;

private:
  void handle_drawing_requests(DrawingRequests& requests, bool hidden_color = false);

private:
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

  void clear_drawing_requests(DrawingRequests& requests);

private:
  VideoSystem& video_system;

  /// the transform stack
  std::vector<Transform> transformstack;
  /// the currently active transform
  Transform transform;

  std::vector<Blend> blend_stack;
  Blend blend_mode;

  DrawingRequests drawing_requests;
  DrawingRequests lightmap_requests;

  DrawingRequests* requests;
  Color ambient_color;

  Target target;
  std::vector<Target> target_stack;

  /* obstack holding the memory of the drawing requests */
  struct obstack obst;

  bool screenshot_requested; /**< true if a screenshot should be taken after the next frame has been rendered */

private:
  DrawingContext(const DrawingContext&);
  DrawingContext& operator=(const DrawingContext&);
};

#endif

/* EOF */
