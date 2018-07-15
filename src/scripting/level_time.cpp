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

LevelTime::LevelTime(::LevelTime* level_time_)
  : level_time(level_time_)
{ }

void LevelTime::start()
{
  level_time->start();
}

void LevelTime::stop()
{
  level_time->stop();
}

float LevelTime::get_time() const
{
  return level_time->get_time();
}

void LevelTime::set_time(float time_left)
{
  level_time->set_time(time_left);
}

}

/* EOF */
