//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_SQUIRREL_SQUIRREL_VIRTUAL_MACHINE_HPP
#define HEADER_SUPERTUX_SQUIRREL_SQUIRREL_VIRTUAL_MACHINE_HPP

#include <squirrel.h>

#include "util/currenton.hpp"

class SquirrelVirtualMachine final : public Currenton<SquirrelVirtualMachine>
{
public:
  SquirrelVirtualMachine(bool enable_debugger);
  ~SquirrelVirtualMachine();

  void update_debugger();

  HSQUIRRELVM get_vm() const { return m_vm; }

private:
  HSQUIRRELVM m_vm;

private:
  SquirrelVirtualMachine(const SquirrelVirtualMachine&) = delete;
  SquirrelVirtualMachine& operator=(const SquirrelVirtualMachine&) = delete;
};

#endif

/* EOF */
