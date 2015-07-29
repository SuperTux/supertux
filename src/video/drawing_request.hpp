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

#include "math/rectf.hpp"
#include "math/sizef.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"
#include "video/glutil.hpp"

class Surface;

enum RequestType
{
  SURFACE, SURFACE_PART, TEXT, GRADIENT, FILLRECT, INVERSEELLIPSE, DRAW_LIGHTMAP, GETLIGHT
};

struct DrawingRequestData
{
  virtual ~DrawingRequestData()
  {}
};

struct SurfaceRequest : public DrawingRequestData
{
  SurfaceRequest() :
    surface()
  {}

  const Surface* surface;

private:
  SurfaceRequest(const SurfaceRequest&) = delete;
  SurfaceRequest& operator=(const SurfaceRequest&) = delete;
};

struct SurfacePartRequest : public DrawingRequestData
{
  SurfacePartRequest() :
    surface(),
    srcrect(),
    dstsize()
  {}

  const Surface* surface;
  Rectf srcrect;
  Sizef dstsize;

private:
  SurfacePartRequest(const SurfacePartRequest&) = delete;
  SurfacePartRequest& operator=(const SurfacePartRequest&) = delete;
};

struct TextRequest : public DrawingRequestData
{
  TextRequest() :
    font(),
    text(),
    alignment()
  {}

  const Font* font;
  std::string text;
  FontAlignment alignment;

private:
  TextRequest(const TextRequest&);
  TextRequest& operator=(const TextRequest&);
};

struct GradientRequest : public DrawingRequestData
{
  GradientRequest()  :
    top(),
    bottom(),
    size(),
    direction(),
    region()
  {}

  Color top;
  Color bottom;
  Vector size;
  GradientDirection direction;
  Rectf region;
};

struct FillRectRequest : public DrawingRequestData
{
  FillRectRequest() :
    color(),
    size(),
    radius()
  {}

  Color  color;
  Vector size;
  float  radius;
};

struct InverseEllipseRequest : public DrawingRequestData
{
  InverseEllipseRequest() :
    color(),
    size()
  {}

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

  DrawingRequestData* request_data;

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

struct GetLightRequest : public DrawingRequestData
{
  GetLightRequest() : color_ptr() {}

  Color* color_ptr;

private:
  GetLightRequest(const GetLightRequest&) = delete;
  GetLightRequest& operator=(const GetLightRequest&) = delete;
};

#endif

/* EOF */
