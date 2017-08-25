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

#ifndef HEADER_SUPERTUX_SCRIPTING_LEVEL_TIME_HPP
#define HEADER_SUPERTUX_SCRIPTING_LEVEL_TIME_HPP

#ifndef SCRIPTING_API
#include "scripting/sqrat_object.hpp"
class LevelTime;
#endif

namespace scripting {

#ifdef SCRIPTING_API
class LevelTime
#else
class LevelTime: SQRatObject<LevelTime>
#endif
{
public:
#ifndef SCRIPTING_API
  LevelTime(::LevelTime* level_time);
  ~LevelTime();
  static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
  {
    squirrelClass.Func("start", &LevelTime::start);
    squirrelClass.Func("stop", &LevelTime::stop);
    squirrelClass.Func("get_time", &LevelTime::get_time);
    squirrelClass.Func("set_time", &LevelTime::set_time);
  }
#endif

  /**
   * Resumes the countdown
   */
  void start();

  /**
   * Pauses the countdown
   */
  void stop();

  /**
   * Returns the number of seconds left on the clock
   */
  float get_time() const;

  /**
   * Changes the number of seconds left on the clock
   */
  void set_time(float time_left);

#ifndef SCRIPTING_API
  ::LevelTime* level_time;

private:
  LevelTime(const LevelTime&);
  LevelTime& operator=(const LevelTime&);
#endif
};

}

#endif

/* EOF */
