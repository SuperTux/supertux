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

#ifndef HEADER_SUPERTUX_SCRIPTING_PLATFORM_HPP
#define HEADER_SUPERTUX_SCRIPTING_PLATFORM_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Platform;
#endif

namespace scripting {

/**
 * @summary A ""Platform"" that was given a name can be controlled by scripts.
            It moves along a specified path.
 * @instances A ""Platform"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Platform final
#ifndef SCRIPTING_API
  : public GameObject<::Platform>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  Platform(const Platform&) = delete;
  Platform& operator=(const Platform&) = delete;
#endif

public:
  /**
   * Moves the platform until at given node, then stops.
   * @param int $node_no
   */
  void goto_node(int node_no);

  /**
   * Starts moving the platform automatically.
   */
  void start_moving();

  /**
   * Stops moving the platform.
   */
  void stop_moving();

  /**
   * Returns the current sprite action.
   */
  std::string get_action() const;
  /**
   * Sets the sprite action.
   * @param string $action
   * @param string $loops
   */
  void set_action(const std::string& action, int loops);
};

} // namespace scripting

#endif

/* EOF */
