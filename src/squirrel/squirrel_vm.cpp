//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//                2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "squirrel/squirrel_vm.hpp"

#include <stdexcept>

SquirrelVM::SquirrelVM() :
  m_vm()
{
  m_vm = sq_open(64);
  if (m_vm == nullptr)
    throw std::runtime_error("Couldn't initialize squirrel vm");
}

SquirrelVM::~SquirrelVM()
{
#ifdef ENABLE_SQDBG
  if (debugger != nullptr) {
    sq_rdbg_shutdown(debugger);
    debugger = nullptr;
  }
#endif

  sq_close(m_vm);
}

/* EOF */
