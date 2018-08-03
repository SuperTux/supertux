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

#include <string>

#include "math/rectf.hpp"
#include "math/sizef.hpp"
#include "math/vector.hpp"
#include "video/color.hpp"
#include "video/drawing_context.hpp"
#include "video/font.hpp"

class Surface;

enum RequestType
{
  TEXTURE, TEXT, GRADIENT, FILLRECT, INVERSEELLIPSE, GETLIGHT, LINE, TRIANGLE
};

struct DrawingRequest
{
  RequestType type;

  int layer;
  DrawingEffect drawing_effect;
  float alpha;
  Blend blend;
  float angle;

  DrawingRequest() = delete;
  DrawingRequest(RequestType type_) :
    type(type_),
    layer(),
    drawing_effect(),
    alpha(),
    blend(),
    angle(0.0f)
  {}
  virtual ~DrawingRequest() {}
};

struct TextureRequest : public DrawingRequest
{
  TextureRequest() :
    DrawingRequest(TEXTURE),
    texture(),
    srcrect(),
    dstrect(),
    color(1.0f, 1.0f, 1.0f)
  {}

  const Texture* texture;
  Rectf srcrect;
  Rectf dstrect;
  Color color;

private:
  TextureRequest(const TextureRequest&) = delete;
  TextureRequest& operator=(const TextureRequest&) = delete;
};

struct TextRequest : public DrawingRequest
{
  TextRequest() :
    DrawingRequest(TEXT),
    pos(),
    font(),
    text(),
    alignment(),
    color()
  {}

  Vector pos;
  const Font* font;
  std::string text;
  FontAlignment alignment;
  Color color;

private:
  TextRequest(const TextRequest&);
  TextRequest& operator=(const TextRequest&);
};

struct GradientRequest : public DrawingRequest
{
  GradientRequest()  :
    DrawingRequest(GRADIENT),
    pos(),
    size(),
    top(),
    bottom(),
    direction(),
    region()
  {}

  Vector pos;
  Vector size;
  Color top;
  Color bottom;
  GradientDirection direction;
  Rectf region;
};

struct FillRectRequest : public DrawingRequest
{
  FillRectRequest() :
    DrawingRequest(FILLRECT),
    pos(),
    size(),
    color(),
    radius()
  {}

  Vector pos;
  Vector size;
  Color  color;
  float  radius;
};

struct InverseEllipseRequest : public DrawingRequest
{
  InverseEllipseRequest() :
    DrawingRequest(INVERSEELLIPSE),
    pos(),
    size(),
    color()
  {}

  Vector pos;
  Vector size;
  Color color;
};

struct LineRequest : public DrawingRequest
{
  LineRequest() :
    DrawingRequest(LINE),
    pos(),
    dest_pos(),
    color()
  {}

  Vector pos;
  Vector dest_pos;
  Color color;
};

struct TriangleRequest : public DrawingRequest
{
  TriangleRequest() :
    DrawingRequest(TRIANGLE),
    pos1(),
    pos2(),
    pos3(),
    color()
  {}

  Vector pos1, pos2, pos3;
  Color  color;
};

struct GetLightRequest : public DrawingRequest
{
  GetLightRequest() :
    DrawingRequest(GETLIGHT),
    pos(),
    color_ptr() {}

  Vector pos;
  Color* color_ptr;

private:
  GetLightRequest(const GetLightRequest&) = delete;
  GetLightRequest& operator=(const GetLightRequest&) = delete;
};

#endif

/* EOF */
