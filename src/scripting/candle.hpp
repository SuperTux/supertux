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

#ifndef HEADER_SUPERTUX_SCRIPTING_CANDLE_HPP
#define HEADER_SUPERTUX_SCRIPTING_CANDLE_HPP

#ifndef SCRIPTING_API
class Candle;
#endif

namespace scripting {

class Candle
{
public:
#ifndef SCRIPTING_API
  Candle(::Candle* candle);
  ~Candle();
#endif

  bool get_burning(); /**< returns true if candle is lighted */
  void set_burning(bool burning); /**< true: light candle, false: extinguish candle */

#ifndef SCRIPTING_API
  ::Candle* candle;

private:
  Candle(const Candle&);
  Candle& operator=(const Candle&);
#endif
};

}

#endif

/* EOF */
