//  SuperTux - Sector scripting
//  Copyright (C) 2020 Tobias Markus <tobbi.bugs@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_BADGUY_HPP
#define HEADER_SUPERTUX_SCRIPTING_BADGUY_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class BadGuy;
#endif

namespace scripting {

/**
 * @summary A ""BadGuy"" that was given a name can be controlled by scripts.
 * @instances A ""BadGuy"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class BadGuy
#ifndef SCRIPTING_API
  : virtual public GameObject<::BadGuy>
#endif
{
#ifndef SCRIPTING_API
public:
  BadGuy(UID uid) :
    GameObject<::BadGuy>(uid)
  {}

private:
  BadGuy(const BadGuy&) = delete;
  BadGuy& operator=(const BadGuy&) = delete;
#endif

public:
  /**
   * Kills the badguy.
   */
  void kill();
  /**
   * Kills the badguy by igniting it.
   */
  void ignite();
  /**
   * Sets the badguy's sprite action.
   * @param string $action The sprite action name.
   * @param int $loops The amount of loops the action should repeat for.
   */
  void set_action(const std::string& action, int loops);
  /**
   * Sets the badguy's sprite.
   * @param string $sprite
   */
  void set_sprite(const std::string& sprite);
};

} // namespace scripting

#endif

/* EOF */
