//  SuperTux
//  Copyright (C) 2020 Grzegorz Przybylski <zwatotem@gmail.com>
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

#include "object/decal.hpp"
#include "scripting/decal.hpp"

namespace scripting {

void
Decal::change_sprite(const std::string& sprite)
{
  SCRIPT_GUARD_VOID;
  object.change_sprite(sprite);
}

void
Decal::fade_sprite(const std::string& sprite, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_sprite(sprite, time);
}

void
Decal::fade_in(float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_in(time);
}

void
Decal::fade_out(float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_out(time);
}

void
Decal::set_action(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_action(action);
}

} // namespace scripting

/* EOF */
