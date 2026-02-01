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

#pragma once

#include <string>
#include <memory>
#include <variant>

#include "math/rectf.hpp"
#include "math/sizef.hpp"
#include "math/vector.hpp"
#include "video/blend.hpp"
#include "video/color.hpp"
#include "video/drawing_transform.hpp"
#include "video/font.hpp"
#include "video/gradient.hpp"

class Surface;

struct TextureRequest
{
  const Texture* texture;
  const Texture* displacement_texture;
  std::vector<Rectf> srcrects;
  std::vector<Rectf> dstrects;
  std::vector<float> angles;
  Color color;
};

struct GradientRequest
{
  Vector pos;
  Vector size;
  Color top;
  Color bottom;
  GradientDirection direction;
  Rectf region;
};

struct FillRectRequest
{
  Rectf rect;
  Color color;
  float radius;
  int blur;
};

struct InverseEllipseRequest
{
  Vector pos;
  Vector size;
  Color color;
};

struct LineRequest
{
  Vector pos;
  Vector dest_pos;
  Color color;
};

struct TriangleRequest
{
  Vector pos1, pos2, pos3;
  Color  color;
};

struct GetPixelRequest
{
  Vector pos;
  std::shared_ptr<Color> color_ptr;
};

struct DrawingRequest
{
  using RequestVariant =
    std::variant<TextureRequest,
                 GradientRequest,
                 FillRectRequest,
                 InverseEllipseRequest,
                 LineRequest,
                 TriangleRequest,
                 GetPixelRequest>;
  int layer;
  Flip flip;
  float alpha;
  Blend blend;
  const Rect viewport;
  RequestVariant request;

  DrawingRequest() = delete;
  DrawingRequest(const DrawingTransform& transform) :
    layer(),
    flip(transform.flip),
    alpha(transform.alpha),
    blend(),
    viewport(transform.viewport),
    request()
  {}
  ~DrawingRequest() {}

};
