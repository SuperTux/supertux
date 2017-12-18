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

#ifndef HEADER_SUPERTUX_SCRIPTING_SCRIPTING_HPP
#define HEADER_SUPERTUX_SCRIPTING_SCRIPTING_HPP

#include <string>
#include <squirrel.h>
#include <sqrat.h>

#include "util/currenton.hpp"

namespace scripting {

extern HSQUIRRELVM global_vm;

class Scripting : public Currenton<Scripting>
{
public:
  Scripting(bool enable_debugger);
  ~Scripting();

  void update_debugger();
  void register_global_constants(HSQUIRRELVM v);
  void register_global_functions(HSQUIRRELVM v);
  void register_scripting_classes(HSQUIRRELVM v);
  template<class T>
  void register_scripting_class(HSQUIRRELVM v, const std::string& name);

private:
  Scripting(const Scripting&) = delete;
  Scripting& operator=(const Scripting&) = delete;
};

} // namespace scripting

#endif

/* EOF */
