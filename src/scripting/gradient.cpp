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

#include "object/gradient.hpp"

namespace scripting {

void
Gradient::set_direction(const std::string& direction)
{
  SCRIPT_GUARD_VOID;
  object.set_direction(direction);
}

std::string
Gradient::get_direction() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_direction_string();
}

void
Gradient::set_color1(float red, float green, float blue)
{
  SCRIPT_GUARD_VOID;
  object.set_gradient(Color(red, green, blue), object.get_gradient_bottom());
}

void
Gradient::set_color2(float red, float green, float blue)
{
  SCRIPT_GUARD_VOID;
  object.set_gradient(object.get_gradient_top(), Color(red, green, blue));
}

void
Gradient::set_colors(float red1, float green1, float blue1, float red2, float green2, float blue2)
{
  SCRIPT_GUARD_VOID;
  object.set_gradient(Color(red1, green1, blue1), Color(red2, green2, blue2));
}

void
Gradient::fade_color1(float red, float green, float blue, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_gradient(Color(red, green, blue), object.get_gradient_bottom(), time);
}

void
Gradient::fade_color2(float red, float green, float blue, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_gradient(object.get_gradient_top(), Color(red, green, blue), time);
}

void
Gradient::fade_colors(float red1, float green1, float blue1, float red2, float green2, float blue2, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_gradient(Color(red1, green1, blue1), Color(red2, green2, blue2), time);
}

void
Gradient::swap_colors()
{
  SCRIPT_GUARD_VOID;
  object.set_gradient(object.get_gradient_bottom(), object.get_gradient_top());
}

} // namespace scripting

/* EOF */
