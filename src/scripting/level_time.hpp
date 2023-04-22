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
#include "scripting/game_object.hpp"

class LevelTime;
#endif

namespace scripting {

/**
 * @summary A ""LevelTime"" that was given a name can be controlled by scripts.
 * @instances A ""LevelTime"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class LevelTime final
#ifndef SCRIPTING_API
  : public GameObject<::LevelTime>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  LevelTime(const LevelTime&) = delete;
  LevelTime& operator=(const LevelTime&) = delete;
#endif

public:
  /**
   * Resumes the countdown (assuming it isn't already started, in which case it does nothing).
   */
  void start();

  /**
   * Pauses the countdown (assuming it isn't already stopped, in which case it does nothing).
   */
  void stop();

  /**
   * Returns the number of seconds left on the clock.
   */
  float get_time() const;

  /**
   * Sets the number of seconds left on the clock.
   * @param float $time_left
   */
  void set_time(float time_left);
};

} // namespace scripting

#endif

/* EOF */
