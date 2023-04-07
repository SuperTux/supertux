//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_SPOTLIGHT_HPP
#define HEADER_SUPERTUX_SCRIPTING_SPOTLIGHT_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class Spotlight;
#endif

namespace scripting {

class Spotlight final
#ifndef SCRIPTING_API
  : public GameObject<::Spotlight>
#endif
{
#ifndef SCRIPTING_API
private:
  using GameObject::GameObject;

private:
  Spotlight(const Spotlight&) = delete;
  Spotlight& operator=(const Spotlight&) = delete;
#endif

public:
  void set_enabled(bool enabled);
  bool is_enabled();

  void set_direction(std::string direction);

  void set_angle(float angle);
  void fade_angle(float angle, float time);
  void ease_angle(float angle, float time, std::string easing);

  void set_speed(float speed);
  void fade_speed(float speed, float time);
  void ease_speed(float speed, float time, std::string easing);

  void set_color_rgba(float r, float g, float b, float a);
  void fade_color_rgba(float r, float g, float b, float a, float time);
  void ease_color_rgba(float r, float g, float b, float a, float time, std::string easing);
};

} // namespace scripting

#endif

/* EOF */
