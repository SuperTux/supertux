//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "scripting/gradient.hpp"

#include <physfs.h>

#include "object/gradient.hpp"

namespace scripting {

Gradient::Gradient(::Gradient* parent) :
  gradient(parent)
{
}

Gradient::~Gradient()
{
}

void
Gradient::set_direction(const std::string& direction)
{
  if(direction == "horizontal")
    gradient->set_direction(GradientDirection::HORIZONTAL);
  else if(direction == "vertical")
    gradient->set_direction(GradientDirection::VERTICAL);
  else if(direction == "horizontal_sector")
    gradient->set_direction(GradientDirection::HORIZONTAL_SECTOR);
  else if(direction == "vertical_sector")
    gradient->set_direction(GradientDirection::VERTICAL_SECTOR);
  else
    log_info << "Invalid direction for gradient \"" << direction << "\"";
}

std::string
Gradient::get_direction() const
{
  auto direction = gradient->get_direction();

  if(direction == GradientDirection::HORIZONTAL)
    return "horizontal";
  if(direction == GradientDirection::VERTICAL)
    return "vertical";
  if(direction == GradientDirection::HORIZONTAL_SECTOR)
    return "horizontal_sector";
  if(direction == GradientDirection::VERTICAL_SECTOR)
    return "vertical_sector";

  return NULL;
}

void
Gradient::set_color1(float red, float green, float blue)
{
  gradient->set_gradient(Color(red, green, blue), gradient->get_gradient_bottom());
}

void
Gradient::set_color2(float red, float green, float blue)
{
  gradient->set_gradient(gradient->get_gradient_top(), Color(red, green, blue));
}

void
Gradient::swap_colors()
{
  gradient->set_gradient(gradient->get_gradient_bottom(), gradient->get_gradient_top());
}

}

/* EOF */
