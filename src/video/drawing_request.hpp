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

#ifndef HEADER_SUPERTUX_VIDEO_DRAWING_REQUEST_HPP
#define HEADER_SUPERTUX_VIDEO_DRAWING_REQUEST_HPP

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/font.hpp"
#include "video/glutil.hpp"

class Surface;

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

enum Target {
  NORMAL, LIGHTMAP
};

enum RequestType
{
  SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT, INVERSEELLIPSE, DRAW_LIGHTMAP, GETLIGHT
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
  Color  color;
  Vector size;
  float  radius;
};

struct InverseEllipseRequest
{
  Color  color;
  Vector size;
};

struct DrawingRequest
{
  Target target;
  RequestType type;
  Vector pos;

  int layer;
  DrawingEffect drawing_effect;
  float alpha;
  Blend blend;
  float angle;
  Color color;

  void* request_data;

  DrawingRequest() :
    target(),
    type(),
    pos(),
    layer(),
    drawing_effect(),
    alpha(),
    blend(),
    angle(0.0f),
    color(1.0f, 1.0f, 1.0f, 1.0f),
    request_data()
  {}

  bool operator<(const DrawingRequest& other) const
  {
    return layer < other.layer;
  }
};

struct GetLightRequest
{
  Color* color_ptr;
};

#endif

