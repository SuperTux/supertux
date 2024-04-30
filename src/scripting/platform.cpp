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

#include "object/platform.hpp"
#include "scripting/platform.hpp"

namespace scripting {

void
Platform::goto_node(int node_no)
{
  SCRIPT_GUARD_VOID;
  object.goto_node(node_no);
}

void
Platform::set_node(int node_no)
{
  SCRIPT_GUARD_VOID;
  object.jump_to_node(node_no, /* instantaneous = */ true);
}

void
Platform::start_moving()
{
  SCRIPT_GUARD_VOID;
  object.start_moving();
}

void
Platform::stop_moving()
{
  SCRIPT_GUARD_VOID;
  object.stop_moving();
}

std::string
Platform::get_action() const
{
  SCRIPT_GUARD_DEFAULT;
  return object.get_action();
}

void
Platform::set_action(const std::string& action, int loops)
{
  SCRIPT_GUARD_VOID;
  object.set_action(action, loops);
}

} // namespace scripting

/* EOF */
