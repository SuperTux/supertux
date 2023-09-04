//  SuperTux
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

#include "scripting/badguy.hpp"

#include <physfs.h>

#include "badguy/badguy.hpp"

namespace scripting {

void
BadGuy::kill()
{
  SCRIPT_GUARD_VOID;
  object.kill_fall();
}

void
BadGuy::ignite()
{
  SCRIPT_GUARD_VOID;
  if(!object.is_flammable() || object.is_ignited())
  {
    return;
  }

  object.ignite();
}

void
BadGuy::set_action(const std::string& action, int loops)
{
  SCRIPT_GUARD_VOID;
  object.set_action(action, loops);
}

void
BadGuy::set_sprite(const std::string& sprite)
{
  SCRIPT_GUARD_VOID;
  object.change_sprite(sprite);
}

} // namespace scripting

/* EOF */
