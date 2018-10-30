//  SuperTux
//  Copyright (C) 2016 Tobias Markus <tobbi.bugs@googlemail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_ROCK_HPP
#define HEADER_SUPERTUX_SCRIPTING_ROCK_HPP

#ifndef SCRIPTING_API
#include "scripting/game_object.hpp"

class Rock;
#endif

namespace scripting {

class Rock final
#ifndef SCRIPTING_API
  : public GameObject<::Rock>
#endif
{
#ifndef SCRIPTING_API
public:
  using GameObject::GameObject;

private:
  Rock(const Rock&) = delete;
  Rock& operator=(const Rock&) = delete;
#endif
};

} // namespace scripting

#endif

/* EOF */
