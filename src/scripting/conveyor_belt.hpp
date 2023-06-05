//  SuperTux
//  Copyright (C) 2022 Raoul1808 <raoulthegeek@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_CONVEYOR_BELT_HPP
#define HEADER_SUPERTUX_SCRIPTING_CONVEYOR_BELT_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class ConveyorBelt;
#endif

namespace scripting
{

/**
 * @summary A ""ConveyorBelt"" that was given a name can be controlled by scripts.
 * @instances A ""ConveyorBelt"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class ConveyorBelt final
#ifndef SCRIPTING_API
        : public GameObject<::ConveyorBelt>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  ConveyorBelt(const ConveyorBelt&) = delete;
  ConveyorBelt& operator=(const ConveyorBelt&) = delete;
#endif

public:
  /**
   * Starts the conveyor belt
   */
  void start();

  /**
   * Stops the conveyor belt
   */
  void stop();

  /**
   * Makes the conveyor shift objects to the left
   */
  void move_left();

  /**
   * Makes the conveyor shift objects to the right
   */
  void move_right();

  /**
   * Change the shifting speed of the conveyor
   * @param float $target_speed
   */
  void set_speed(float target_speed);
};

} // namespace scripting

#endif

/* EOF */
