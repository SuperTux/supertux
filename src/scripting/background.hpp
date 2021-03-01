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

#ifndef HEADER_SUPERTUX_SCRIPTING_BACKGROUND_HPP
#define HEADER_SUPERTUX_SCRIPTING_BACKGROUND_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Background;
#endif

namespace scripting {

class Background final
#ifndef SCRIPTING_API
  : public GameObject<::Background>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Background(const Background&) = delete;
  Background& operator=(const Background&) = delete;
#endif

public:
  void set_image(const std::string& image);
  void set_images(const std::string& top_image, const std::string& middle_image,
                             const std::string& bottom_image);
  void set_speed(float speed);

  float get_color_red();
  float get_color_green();
  float get_color_blue();
  float get_color_alpha();
  void set_color(float red, float green, float blue, float alpha);
  void fade_color(float red, float green, float blue, float alpha, float time);
};

} // namespace scripting

#endif

/* EOF */
