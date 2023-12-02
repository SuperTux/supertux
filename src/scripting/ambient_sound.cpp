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

#include "scripting/ambient_sound.hpp"

#include "object/ambient_sound.hpp"

namespace scripting {

void
AmbientSound::set_pos(float x, float y)
{
  SCRIPT_GUARD_VOID;
  object.set_pos(x, y);
}

float
AmbientSound::get_pos_x() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos_x();
}

float
AmbientSound::get_pos_y() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_pos_y();
}

} // namespace scripting

/* EOF */
