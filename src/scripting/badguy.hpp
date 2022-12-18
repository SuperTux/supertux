//  SuperTux - Sector scripting
//  Copyright (C) 2020 Tobias Markus <tobbi.bugs@gmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_BADGUY_HPP
#define HEADER_SUPERTUX_SCRIPTING_BADGUY_HPP

#ifndef SCRIPTING_API
#include <string>

#include "scripting/game_object.hpp"

class BadGuy;
#endif

namespace scripting {

class BadGuy
#ifndef SCRIPTING_API
  : virtual public GameObject<::BadGuy>
#endif
{
#ifndef SCRIPTING_API
public:
  BadGuy(UID uid) :
    GameObject<::BadGuy>(uid)
  {}

private:
  BadGuy(const BadGuy&) = delete;
  BadGuy& operator=(const BadGuy&) = delete;
#endif

public:
  void kill();
  void ignite();
  void set_action(const std::string& action, int loops);
  void set_sprite(const std::string& sprite);
};

} // namespace scripting

#endif

/* EOF */
