//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2017 M. Teufel <mteufel@supertux.org>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_LIT_OBJECT_HPP
#define HEADER_SUPERTUX_SCRIPTING_LIT_OBJECT_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class LitObject;
#endif

namespace scripting {

/**
 * @summary A ""LitObject"" that was given a name can be controlled by scripts.
 * @instances A ""LitObject"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class LitObject final
#ifndef SCRIPTING_API
  : public GameObject<::LitObject>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  LitObject(const LitObject&) = delete;
  LitObject& operator=(const LitObject&) = delete;
#endif

public:
  /**
   * Returns the current sprite action.
   */
  std::string get_action() const;
  /**
   * Sets the sprite action.
   * @param string $action
   */
  void set_action(const std::string& action);

  /**
   * Returns the current light sprite action.
   */
  std::string get_light_action() const;
  /**
   * Sets the light sprite action.
   * @param string $action
   */
  void set_light_action(const std::string& action);
};

} // namespace scripting

#endif

/* EOF */
