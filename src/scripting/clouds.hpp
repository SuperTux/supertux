//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_CLOUDS_HPP
#define HEADER_SUPERTUX_SCRIPTING_CLOUDS_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class CloudParticleSystem;
#endif

namespace scripting {

/**
 * @summary A ""CloudParticleSystem"" that was given a name can be controlled by scripts.
 * @instances A ""CloudParticleSystem"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Clouds final
#ifndef SCRIPTING_API
  : public GameObject<::CloudParticleSystem>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Clouds(const Clouds&) = delete;
  Clouds& operator=(const Clouds&) = delete;
#endif

public:
  /**
   * Enables/disables the system.
   * @param bool $enable
   */
  void set_enabled(bool enable);
  /**
   * Returns ""true"" if the system is enabled.
   */
  bool get_enabled() const;

  /**
   * Smoothly changes the rain speed to the given value in ""time"" seconds.
   * @param float $speed
   * @param float $time
   */
  void fade_speed(float speed, float time);

  /**
   * Smoothly changes the amount of particles to the given value in ""time"" seconds.
   * @param int $amount
   * @param float $time
   * @param float $time_between
   */
  void fade_amount(int amount, float time, float time_between);

  /**
   * Smoothly changes the amount of particles to the given value in ""time"" seconds.
   * @param int $amount
   * @param float $time
   */
  void set_amount(int amount, float time);
};

} // namespace scripting

#endif

/* EOF */
