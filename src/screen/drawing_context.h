//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Matthias Braun <matze@braunis.de
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

#include "SDL.h"

#include "vector.h"
#include "screen.h"

class Surface;
class Font;

// some constants for predefined layer values
enum {
  LAYER_BACKGROUND0 = -300,
  LAYER_BACKGROUND1 = -200,
  LAYER_BACKGROUNDTILES = -100,
  LAYER_TILES = 0,
  LAYER_OBJECTS = 100,
  LAYER_FOREGROUNDTILES = 200,
  LAYER_FOREGROUND0 = 300,
  LAYER_FOREGROUND1 = 400,
  LAYER_GUI         = 500
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

  /** Adds a drawing request for a surface into the request list */
  void draw_surface(const Surface* surface, const Vector& position, int layer,
      Uint32 drawing_effect = NONE_EFFECT);
  /** Adds a drawing request for part of a surface */
  void draw_surface_part(const Surface* surface, const Vector& source,
      const Vector& size, const Vector& dest, int layer,
      Uint32 drawing_effect = NONE_EFFECT);
  /** draws a text */
  void draw_text(Font* font, const std::string& text, const Vector& position,
      int layer);
  /** draws aligned text */
  void draw_text_center(Font* font, const std::string& text,
      const Vector& position, int layer);
  /** draws a color gradient onto the whole screen */  
  void draw_gradient(Color from, Color to, int layer);
  /** fills a rectangle */
  void draw_filled_rect(const Vector& topleft, const Vector& size,
          Color color, int layer);
  /** draws an one-pixel line */
  void draw_line(const Vector& topleft, const Vector& downright,
          Color color, int layer);
  
  /** Processes all pending drawing requests and flushes the list */
  void do_drawing();

  const Vector& get_translation() const
  { return transform.translation; }
  void set_translation(const Vector& newtranslation)
  { transform.translation = newtranslation; }

  void push_transform();
  void pop_transform();

private:
  class Transform
  {
  public:
    Vector translation; // only translation for now...

    Vector apply(const Vector& v) const
    {
      return v - translation;
    }
  };

  /// the transform stack
  std::vector<Transform> transformstack;
  /// the currently active transform
  Transform transform;

  enum RequestType
  {
    SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT, LINE
  };

  struct SurfacePartRequest
  {
    const Surface* surface;
    Vector source, size;
  };

  struct TextRequest
  {
    Font* font;
    std::string text;
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

  struct LineRequest
  {
    Color color;
    Vector botright;
  };

  struct DrawingRequest
  {
    int layer;
    Uint32 drawing_effect;

    RequestType type;
    Vector pos;

    void* request_data;

    bool operator<(const DrawingRequest& other) const
    {
      return layer < other.layer;
    }
  };

  void draw_surface_part(DrawingRequest& request);
  void draw_text(DrawingRequest& request);
  void draw_gradient(DrawingRequest& request);
  void draw_filled_rect(DrawingRequest& request);
  void draw_line(DrawingRequest& request);
  
  typedef std::vector<DrawingRequest> DrawingRequests;
  DrawingRequests drawingrequests;
};

#endif /*SUPERTUX_DRAWINGCONTEXT_H*/

