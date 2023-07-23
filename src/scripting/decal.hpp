//  SuperTux
//  Copyright (C) 2020 Grzegorz Przybylski <zwatotem@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_DECAL_HPP
#define HEADER_SUPERTUX_SCRIPTING_DECAL_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Decal;
#endif

namespace scripting {

/**
 * @summary A ""Decal"" that was given a name can be controlled by scripts.
 * @instances A ""Decal"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Decal final
#ifndef SCRIPTING_API
  : public GameObject<::Decal>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  Decal(const Decal&) = delete;
  Decal& operator=(const Decal&) = delete;
#endif

public:
  /**
   * Fades the decal sprite to a new one in ""time"" seconds.
   * @param string $sprite
   * @param float $time
   */
  void fade_sprite(const std::string& sprite, float time);
  /**
   * Changes the decal sprite.
   * @param string $sprite
   */
  void change_sprite(const std::string& sprite);
  /**
   * Fades in the decal in ""time"" seconds.
   * @param float $time
   */
  void fade_in(float time);
  /**
   * Fades out the decal in ""time"" seconds.
   * @param float $time
   */
  void fade_out(float time);
  /**
   * Sets the action for this decal
   * @param float $time
   */
  void set_action(const std::string& action);
};

} // namespace scripting

#endif

/* EOF */
