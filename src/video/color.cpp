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

#include <algorithm>
#include <config.h>

#include "video/color.hpp"

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
  alpha(1.0f),
  ultra_violet(0)
{}

Color::Color(float red_, float green_, float blue_, float alpha_, float ultra_violet_) :
  red(red_),
  green(green_),
  blue(blue_),
  alpha(alpha_),
  ultra_violet(ultra_violet_)
{
  assert(0 <= red   && red <= 1.0);
  assert(0 <= green && green <= 1.0);
  assert(0 <= blue  && blue <= 1.0);
  assert(0 <= ultra_violet  && ultra_violet <= 1.0);
}

Color::Color(const std::vector<float>& vals) :
  red(),
  green(),
  blue(),
  alpha(),
  ultra_violet()
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
  if(vals.size() > 3) {
    alpha = vals[3];
  } else {
    alpha = 1.0;
  }

  if(vals.size() > 4) {
    ultra_violet = vals[4];
  } else {
    ultra_violet = 0.0;
  }

  assert(0 <= red   && red <= 1.0);
  assert(0 <= green && green <= 1.0);
  assert(0 <= blue  && blue <= 1.0);
  assert(0 <= ultra_violet  && ultra_violet <= 1.0);
}

Color::Color(const Color visible, const Color hidden) :
  red(hidden.red),
  green(visible.green),
  blue(hidden.blue),
  alpha(visible.alpha),
  ultra_violet(hidden.green)
{

}

bool
Color::operator==(const Color& other) const
{
  return red == other.red && green == other.green && blue == other.blue
    && alpha == other.alpha && ultra_violet == other.ultra_violet;
}

float
Color::greyscale() const
{
  Color visible = get_visible_color();
  return visible.red * 0.30f + visible.green * 0.59f + visible.blue * 0.11f;
}

bool
Color::operator < (const Color& other) const
{
  return greyscale() < other.greyscale();
}

Color
Color::get_visible_color() const
{
  Color result(red, green, blue, alpha);
  result.red = std::min(1.0f, red + ultra_violet * 0.2f);
  result.blue = std::min(1.0f, blue + ultra_violet * 0.3f);
  return result;
}

Color
Color::get_hidden_color() const
{
  return Color(red, ultra_violet, blue, alpha);
}

std::vector<float>
Color::toVector()
{
  std::vector<float> result;
  result.clear();
  result.push_back(red);
  result.push_back(green);
  result.push_back(blue);
  result.push_back(alpha);
  result.push_back(ultra_violet);
  return result;
}

/* EOF */
