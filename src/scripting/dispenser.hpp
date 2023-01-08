//  SuperTux
//  Copyright (C) 2018 Ashish Bhattarai <ashishbhattarai@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_DISPENSER_HPP
#define HEADER_SUPERTUX_SCRIPTING_DISPENSER_HPP

#ifndef SCRIPTING_API

#include "scripting/badguy.hpp"

class Dispenser;
#endif

namespace scripting {

class Dispenser final : public scripting::BadGuy
#ifndef SCRIPTING_API
  , virtual public GameObject<::Dispenser>
#endif
{
#ifndef SCRIPTING_API
public:
  Dispenser(::GameObject* object) :
    GameObject<::BadGuy>(object),
    GameObject<::Dispenser>(object),
    BadGuy(object)
  {}

private:
  Dispenser(const Dispenser&) = delete;
  Dispenser& operator=(const Dispenser&) = delete;
#endif

public:
  /**
   * Make the Dispenser start dispensing BadGuys
   */
  void activate();
  /**
   * Make the Dispenser stop dispensing BadGuys
   */
  void deactivate();
};

} // namespace scripting

#endif

/* EOF */
