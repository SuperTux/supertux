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

#include "video/color.hpp"

#include <assert.h>

const Color Color::BLACK(0.0, 0.0, 0.0);
const Color Color::RED(1.0, 0.0, 0.0);
const Color Color::GREEN(0.0, 1.0, 0.0);
const Color Color::BLUE(0.0, 0.0, 1.0);
const Color Color::CYAN(0.0, 1.0, 1.0);
const Color Color::MAGENTA(1.0, 0.0, 1.0);
const Color Color::YELLOW(1.0, 1.0, 0.0);
const Color Color::WHITE(1.0, 1.0, 1.0);

Color::Color() :
  red(0),
  green(0),
  blue(0),
  alpha(1.0f)
{}

Color::Color(float red_, float green_, float blue_, float alpha_) :
  red(red_),
  green(green_),
  blue(blue_),
  alpha(alpha_)
{
  assert(0 <= red   && red <= 1.0f);
  assert(0 <= green && green <= 1.0f);
  assert(0 <= blue  && blue <= 1.0f);
}

Color::Color(const std::vector<float>& vals) :
  red(),
  green(),
  blue(),
  alpha()
{
  if (vals.size() < 3) {
    red = 0;
    green = 0;
    blue = 0;
    alpha = 0;
    return;
  }
  red   = vals[0];
  green = vals[1];
  blue  = vals[2];
  if (vals.size() > 3)
    alpha = vals[3];
  else
    alpha = 1.0;
  assert(0 <= red   && red <= 1.0f);
  assert(0 <= green && green <= 1.0f);
  assert(0 <= blue  && blue <= 1.0f);
}

bool
Color::operator==(const Color& other) const
{
  return red == other.red && green == other.green && blue == other.blue
    && alpha == other.alpha;
}

bool
Color::operator!=(const Color& other) const
{
  return !(operator==(other));
}

float
Color::greyscale() const
{
  return red * 0.30f + green * 0.59f + blue * 0.11f;
}

Color
Color::multiply_linearly(float v) const
{
  // For the approximate sRGB conversion, it is sufficient to apply the exponent
  // to v.
  v = add_gamma(v);
  return Color(red * v, green * v, blue * v, alpha);
}

bool
Color::operator < (const Color& other) const
{
  return greyscale() < other.greyscale();
}

std::vector<float>
Color::toVector() const
{
  std::vector<float> result;
  result.clear();
  result.push_back(red);
  result.push_back(green);
  result.push_back(blue);
  result.push_back(alpha);
  return result;
}

/* EOF */
