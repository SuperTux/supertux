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

#include "scripting/spotlight.hpp"

#include "object/spotlight.hpp"

namespace scripting {


void
Spotlight::set_enabled(bool enabled)
{
  SCRIPT_GUARD_VOID;
  object.set_enabled(enabled);
}

bool
Spotlight::is_enabled()
{
  SCRIPT_GUARD_DEFAULT;
  return object.is_enabled();
}

void
Spotlight::set_direction(std::string direction)
{
  SCRIPT_GUARD_VOID;
  object.set_direction(::Spotlight::Direction_from_string(direction));
}

void
Spotlight::set_speed(float speed)
{
  SCRIPT_GUARD_VOID;
  object.set_speed(speed);
}

void
Spotlight::fade_speed(float speed, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_speed(time, speed);
}

void
Spotlight::ease_speed(float speed, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_speed(time, speed, EasingMode_from_string(easing));
}

void
Spotlight::set_angle(float angle)
{
  SCRIPT_GUARD_VOID;
  object.set_angle(angle);
}

void
Spotlight::fade_angle(float angle, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_angle(time, angle);
}

void
Spotlight::ease_angle(float angle, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_angle(time, angle, EasingMode_from_string(easing));
}

void
Spotlight::set_color_rgba(float r, float g, float b, float a)
{
  SCRIPT_GUARD_VOID;
  object.set_color(Color(r, g, b, a));
}

void
Spotlight::fade_color_rgba(float r, float g, float b, float a, float time)
{
  SCRIPT_GUARD_VOID;
  object.ease_color(time, Color(r, g, b, a));
}

void
Spotlight::ease_color_rgba(float r, float g, float b, float a, float time, std::string easing)
{
  SCRIPT_GUARD_VOID;
  object.ease_color(time, Color(r, g, b, a), EasingMode_from_string(easing));
}


} // namespace scripting

/* EOF */
