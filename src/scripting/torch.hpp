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

#ifndef HEADER_SUPERTUX_SCRIPTING_TORCH_HPP
#define HEADER_SUPERTUX_SCRIPTING_TORCH_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Torch;
#endif

namespace scripting {

/**
 * @summary A ""Torch"" that was given a name can be controlled by scripts.
 * @instances A ""Torch"" is instantiated by placing a definition inside a level.
              It can then be accessed by its name from a script or via ""sector.name"" from the console.
 */
class Torch final
#ifndef SCRIPTING_API
  : public GameObject<::Torch>
#endif
{
public:
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;
private:
  Torch(const Torch&) = delete;
  Torch& operator=(const Torch&) = delete;
#endif

public:
  /**
   * Returns ""true"" if the torch is burning.
   */
  bool get_burning() const;
  /**
   * Switches the burning state of the torch.
   * @param bool $burning
   */
  void set_burning(bool burning);
};

} // namespace scripting

#endif

/* EOF */
