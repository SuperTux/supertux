//  SuperTux
//  Copyright (C) 2018 Ashish Bhattarai <ashishbhattarai@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_DISPENSER_HPP
#define HEADER_SUPERTUX_SCRIPTING_DISPENSER_HPP

#ifndef SCRIPTING_API

#include "scripting/badguy.hpp"

class Dispenser;
#endif

namespace scripting {

/**
 * @summary A ""Dispenser"" that was given a name can be controlled by scripts.
 * @instances A ""Dispenser"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Dispenser final : public scripting::BadGuy
#ifndef SCRIPTING_API
  , virtual public GameObject<::Dispenser>
#endif
{
#ifndef SCRIPTING_API
public:
  Dispenser(UID uid) :
    GameObject<::BadGuy>(uid),
    GameObject<::Dispenser>(uid),
    BadGuy(uid)
  {}

private:
  Dispenser(const Dispenser&) = delete;
  Dispenser& operator=(const Dispenser&) = delete;
#endif

public:
  /**
   * Makes the dispenser start dispensing badguys.
   */
  void activate();
  /**
   * Stops the dispenser from dispensing badguys.
   */
  void deactivate();
};

} // namespace scripting

#endif

/* EOF */
