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

#ifndef HEADER_SUPERTUX_SCRIPTING_THUNDERSTORM_HPP
#define HEADER_SUPERTUX_SCRIPTING_THUNDERSTORM_HPP

#ifndef SCRIPTING_API
#include "scripting/sqrat_object.hpp"
class Thunderstorm;
#endif

namespace scripting {

#ifdef SCRIPTING_API
class Thunderstorm
#else
class Thunderstorm : SQRatObject<Thunderstorm>
#endif
{
public:
#ifndef SCRIPTING_API
  Thunderstorm(::Thunderstorm* thunderstorm);
  ~Thunderstorm();
  static void register_exposed_methods(HSQUIRRELVM v, SQRatClassType squirrelClass)
  {
    squirrelClass.Func("start", &Thunderstorm::start);
    squirrelClass.Func("stop", &Thunderstorm::stop);
    squirrelClass.Func("thunder", &Thunderstorm::thunder);
    squirrelClass.Func("lightning", &Thunderstorm::lightning);
    squirrelClass.Func("flash", &Thunderstorm::flash);
    squirrelClass.Func("electrify", &Thunderstorm::electrify);
  }
#endif

  /**
   * Start playing thunder and lightning at configured interval
   */
  void start();

  /**
   * Stop playing thunder and lightning at configured interval
   */
  void stop();

  /**
   * Play thunder
   */
  void thunder();

  /**
   * Play lightning, i.e. call flash() and electrify()
   */
  void lightning();

  /**
   * Display a nice flash
   */
  void flash();

  /**
   * Electrify water throughout the whole sector for a short time
   */
  void electrify();

#ifndef SCRIPTING_API
  ::Thunderstorm* thunderstorm;

private:
  Thunderstorm(const Thunderstorm&);
  Thunderstorm& operator=(const Thunderstorm&);
#endif
};

}

#endif

/* EOF */
