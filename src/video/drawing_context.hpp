//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_DRAWINGCONTEXT_H
#define SUPERTUX_DRAWINGCONTEXT_H

#include <vector>
#include <string>
#include <stdint.h>

#include <GL/gl.h>
#include <SDL.h>
#include <stdint.h>
#include <memory>

#include "math/vector.hpp"
#include "math/rect.hpp"
#include "surface.hpp"
#include "font.hpp"
#include "color.hpp"

class Surface;
class Texture;

// some constants for predefined layer values
enum {
  LAYER_BACKGROUND0 = -300,
  LAYER_BACKGROUND1 = -200,
  LAYER_BACKGROUNDTILES = -100,
  LAYER_TILES = 0,
  LAYER_OBJECTS = 50,
  LAYER_FLOATINGOBJECTS = 150,
  LAYER_FOREGROUNDTILES = 200,
  LAYER_FOREGROUND0 = 300,
  LAYER_FOREGROUND1 = 400,
  LAYER_HUD = 500,
  LAYER_GUI         = 600
};

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

/**
 * This class provides functions for drawing things on screen. It also
 * maintains a stack of transforms that are applied to graphics.
 */
class DrawingContext
{
public:
  DrawingContext();
  ~DrawingContext();

  /// Adds a drawing request for a surface into the request list.
  void draw_surface(const Surface* surface, const Vector& position,
                    int layer);
  /// Adds a drawing request for a surface into the request list.
  void draw_surface(const Surface* surface, const Vector& position,
                    float angle, const Color& color, const Blend& blend,
                    int layer);
  /// Adds a drawing request for part of a surface.
  void draw_surface_part(const Surface* surface, const Vector& source,
                         const Vector& size, const Vector& dest, int layer);
  /// Draws a text.
  void draw_text(const Font* font, const std::string& text,
                 const Vector& position, FontAlignment alignment, int layer);

  /// Draws text on screen center (feed Vector.x with a 0).
  /// This is the same as draw_text() with a SCREEN_WIDTH/2 position and
  /// alignment set to LEFT_ALLIGN
  void draw_center_text(const Font* font, const std::string& text,
                        const Vector& position, int layer);
  /// Draws a color gradient onto the whole screen */
  void draw_gradient(const Color& from, const Color& to, int layer);
  /// Fills a rectangle.
  void draw_filled_rect(const Vector& topleft, const Vector& size,
                        const Color& color, int layer);
  void draw_filled_rect(const Rect& rect, const Color& color, int layer);

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

  enum Target {
    NORMAL, LIGHTMAP
  };
  void push_target();
  void pop_target();
  void set_target(Target target);
  
  void set_ambient_color( Color new_color );

private:
  class Transform
  {
  public:
    Vector translation;
    DrawingEffect drawing_effect;
    float alpha;

    Transform()
      : drawing_effect(NO_EFFECT), alpha(1.0f)
    { }

    Vector apply(const Vector& v) const
    {
      return v - translation;
    }
  };

  /// the transform stack
  std::vector<Transform> transformstack;
  /// the currently active transform
  Transform transform;

  std::vector<Blend> blend_stack;
  Blend blend_mode;

  enum RequestType
  {
    SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT, LIGHTMAPREQUEST
  };

  struct SurfacePartRequest
  {
    const Surface* surface;
    Vector source, size;
  };

  struct TextRequest
  {
    const Font* font;
    std::string text;
    FontAlignment alignment;
  };

  struct GradientRequest
  {
    Color top, bottom;
    Vector size;
  };

  struct FillRectRequest
  {
    Color color;
    Vector size;
  };

  struct DrawingRequest
  {
    RequestType type;
    Vector pos;

    int layer;
    DrawingEffect drawing_effect;
    float alpha;
    Blend blend;
    float angle;
    Color color;

    void* request_data;

    DrawingRequest()
      : angle(0.0f),
        color(1.0f, 1.0f, 1.0f, 1.0f)
    {}

    bool operator<(const DrawingRequest& other) const
    {
      return layer < other.layer;
    }
  };

  typedef std::vector<DrawingRequest> DrawingRequests;

  void handle_drawing_requests(DrawingRequests& requests);
  void draw_surface_part(DrawingRequest& request);
  void draw_text(DrawingRequest& request);
  void draw_text_center(DrawingRequest& request);
  void draw_gradient(DrawingRequest& request);
  void draw_filled_rect(DrawingRequest& request);
  void draw_lightmap(DrawingRequest& request);

  DrawingRequests drawing_requests;
  DrawingRequests lightmap_requests;

  DrawingRequests* requests;
  Color ambient_color;

  SDL_Surface* screen;
  Target target;
  std::vector<Target> target_stack;
  Texture* lightmap;
  int lightmap_width, lightmap_height;
  float lightmap_uv_right, lightmap_uv_bottom;
};

#endif
