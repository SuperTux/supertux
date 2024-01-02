//  SuperTux - Sector scripting
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_PARTICLESYSTEM_HPP
#define HEADER_SUPERTUX_SCRIPTING_PARTICLESYSTEM_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class ParticleSystem;
#endif

namespace scripting {

/**
 * @summary A ""ParticleSystem"" that was given a name can be controlled by scripts.
 * @instances A ""ParticleSystem"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class ParticleSystem final
#ifndef SCRIPTING_API
  : public GameObject<::ParticleSystem>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;
#endif

public:
  /**
   * Enables/disables the system.
   * @param bool $enable
   */
  void set_enabled(bool enable);
  /**
   * Returns whether the system is enabled.
   */
  bool get_enabled() const;
};

} // namespace scripting

#endif

/* EOF */
