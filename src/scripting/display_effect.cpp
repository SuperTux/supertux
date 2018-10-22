//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/display_effect.hpp"

#include "object/display_effect.hpp"

namespace scripting {

void
DisplayEffect::fade_out(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_out(fadetime);
}

void
DisplayEffect::fade_in(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.fade_in(fadetime);
}

void
DisplayEffect::set_black(bool enabled)
{
  SCRIPT_GUARD_VOID;
  object.set_black(enabled);
}

bool
DisplayEffect::is_black() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.is_black();
}

void
DisplayEffect::sixteen_to_nine(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.sixteen_to_nine(fadetime);
}

void
DisplayEffect::four_to_three(float fadetime)
{
  SCRIPT_GUARD_VOID;
  object.four_to_three(fadetime);
}

} // namespace scripting

/* EOF */
