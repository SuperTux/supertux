//  SuperTux
//  Copyright (C) 2022 Raoul1808 <raoulthegeek@gmail.com>
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

#include "object/conveyor_belt.hpp"
#include "scripting/conveyor_belt.hpp"

namespace scripting
{

void
ConveyorBelt::start()
{
  SCRIPT_GUARD_VOID;
  object.start();
}

void
ConveyorBelt::stop()
{
  SCRIPT_GUARD_VOID;
  object.stop();
}

void
ConveyorBelt::move_left()
{
  SCRIPT_GUARD_VOID;
  object.move_left();
}

void
ConveyorBelt::move_right()
{
  SCRIPT_GUARD_VOID;
  object.move_right();
}

void
ConveyorBelt::set_speed(float target_speed)
{
  SCRIPT_GUARD_VOID;
  object.set_speed(target_speed);
}

} // namespace scripting

/* EOF */
