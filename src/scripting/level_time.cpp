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

#include "object/level_time.hpp"
#include "scripting/level_time.hpp"

namespace scripting {

void LevelTime::start()
{
  SCRIPT_GUARD_VOID;
  object.start();
}

void LevelTime::stop()
{
  SCRIPT_GUARD_VOID;
  object.stop();
}

float LevelTime::get_time() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_time();
}

void LevelTime::set_time(float time_left)
{
  SCRIPT_GUARD_VOID;
  object.set_time(time_left);
}

} // namespace scripting

/* EOF */
