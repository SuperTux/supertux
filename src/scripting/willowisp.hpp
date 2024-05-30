//  SuperTux
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_WILLOWISP_HPP
#define HEADER_SUPERTUX_SCRIPTING_WILLOWISP_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/badguy.hpp"

class WillOWisp;
#endif

namespace scripting {

/**
 * @summary A ""WillOWisp"" that was given a name can be controlled by scripts.
            The WillOWisp can be moved by specifying a path for it.
 * @instances A ""WillOWisp"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class WillOWisp final : public scripting::BadGuy
#ifndef SCRIPTING_API
  , virtual public GameObject<::WillOWisp>
#endif
{
#ifndef SCRIPTING_API
public:
  WillOWisp(const ::GameObject& object) :
    GameObject<::BadGuy>(object),
    GameObject<::WillOWisp>(object),
    BadGuy(object)
  {}

private:
  WillOWisp(const WillOWisp&) = delete;
  WillOWisp& operator=(const WillOWisp&) = delete;
#endif

public:
  /**
   * Moves the WillOWisp along a path until at given node, then stops.
   * @param int $node_idx
   */
  void goto_node(int node_idx);

  /**
   * Sets the state of the WillOWisp.
   * @param string $state One of the following: "stopped", "move_path" (moves along a path),
      "move_path_track" (moves along a path but catches Tux when he is near), "normal" (starts tracking Tux when he is near enough),
      "vanish".
   */
  void set_state(const std::string& state);

  /**
   * Starts following a path.
   */
  void start_moving();
  /**
   * Stops following a path.
   */
  void stop_moving();
};

} // namespace scripting

#endif

/* EOF
   */
