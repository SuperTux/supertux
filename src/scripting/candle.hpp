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
#include "scripting/game_object.hpp"

class Candle;
#endif

namespace scripting {

/**
 * @summary A ""Candle"" that was given a name can be controlled by scripts.
 * @instances A ""Candle"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Candle final
#ifndef SCRIPTING_API
  : public GameObject<::Candle>
#endif
{
public:
#ifndef SCRIPTING_API
  using GameObject::GameObject;

private:
  Candle(const Candle&) = delete;
  Candle& operator=(const Candle&) = delete;
#endif

public:
  /**
   * Returns ""true"" if the candle is lit up.
   */
  bool get_burning() const;
  /**
   * Sets the burning state of the candle.
   * @param bool $burning If ""true"", the candle is lit up. If ""false"", it's extinguished.
   */
  void set_burning(bool burning);
};

} // namespace scripting

#endif

/* EOF */
