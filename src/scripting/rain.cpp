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

#include "math/easing.hpp"
#include "object/rain_particle_system.hpp"
#include "scripting/rain.hpp"

namespace scripting {

void Rain::set_enabled(bool enable)
{
  SCRIPT_GUARD_VOID;
  object.set_enabled(enable);
}

bool Rain::get_enabled() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_enabled();
}

void Rain::fade_speed(float speed, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_speed(speed, time);
}

void Rain::fade_amount(float amount, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_amount(amount, time);
}

void Rain::fade_angle(float angle, float time, const std::string& ease)
{
  SCRIPT_GUARD_VOID;
  object.fade_angle(angle, time, getEasingByName(EasingMode_from_string(ease)));
}

} // namespace scripting

/* EOF */
