//  SuperTux - Sector scripting
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

#ifndef HEADER_SUPERTUX_SCRIPTING_GRADIENT_HPP
#define HEADER_SUPERTUX_SCRIPTING_GRADIENT_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Gradient;
#endif

namespace scripting {

class Gradient final
#ifndef SCRIPTING_API
  : public GameObject<::Gradient>
#endif
{
#ifndef SCRIPTING_API
private:
  using GameObject::GameObject;

private:
  Gradient(const Gradient&) = delete;
  Gradient& operator=(const Gradient&) = delete;
#endif

public:
  void set_direction(const std::string& direction);
  std::string get_direction() const;

  void set_color1(float red, float green, float blue);
  void set_color2(float red, float green, float blue);
  void set_colors(float red1, float green1, float blue1, float red2, float green2, float blue2);
  void fade_color1(float red, float green, float blue, float time);
  void fade_color2(float red, float green, float blue, float time);
  void fade_colors(float red1, float green1, float blue1, float red2, float green2, float blue2, float time);

  void swap_colors();
};

} // namespace scripting

#endif

/* EOF */
