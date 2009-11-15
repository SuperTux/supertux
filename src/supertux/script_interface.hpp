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
#ifndef HEADER_SUPERTUX_SUPERTUX_SCRIPT_INTERFACE_HPP
#define HEADER_SUPERTUX_SUPERTUX_SCRIPT_INTERFACE_HPP

#include <squirrel.h>

/**
 * Objects that want to expose themself to the scripting environment
 * should implement this interface
 */
class ScriptInterface
{
public:
  virtual ~ScriptInterface()
  {}

  virtual void expose(HSQUIRRELVM vm, SQInteger table_idx) = 0;
  virtual void unexpose(HSQUIRRELVM vm, SQInteger table_idx) = 0;
};

#endif
