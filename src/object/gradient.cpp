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

#include "object/gradient.hpp"
#include "supertux/object_factory.hpp"
#include "util/reader.hpp"

Gradient::Gradient() :
  layer(LAYER_BACKGROUND0)
{
}

Gradient::Gradient(const Reader& reader) :
  layer(LAYER_BACKGROUND0)
{
  reader.get("layer", layer);
  std::vector<float> bkgd_top_color, bkgd_bottom_color;
  if(!reader.get("top_color", bkgd_top_color) ||
     !reader.get("bottom_color", bkgd_bottom_color))
    throw std::runtime_error("Must specify top_color and bottom_color in gradient");

  gradient_top = Color(bkgd_top_color);
  gradient_bottom = Color(bkgd_bottom_color);
}

Gradient::~Gradient()
{
}

void
Gradient::update(float)
{
}

void
Gradient::set_gradient(Color top, Color bottom)
{
  gradient_top = top;
  gradient_bottom = bottom;

  if (gradient_top.red > 1.0 || gradient_top.green > 1.0
      || gradient_top.blue > 1.0 || gradient_top.alpha > 1.0)
    log_warning << "top gradient color has values above 1.0" << std::endl;
  if (gradient_bottom.red > 1.0 || gradient_bottom.green > 1.0
      || gradient_bottom.blue > 1.0 || gradient_bottom.alpha > 1.0)
    log_warning << "bottom gradient color has values above 1.0" << std::endl;
}

void
Gradient::draw(DrawingContext& context)
{
  context.push_transform();
  context.set_translation(Vector(0, 0));
  context.draw_gradient(gradient_top, gradient_bottom, layer);
  context.pop_transform();
}

IMPLEMENT_FACTORY(Gradient, "gradient");

/* EOF */
