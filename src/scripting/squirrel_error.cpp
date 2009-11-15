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
#include <config.h>

#include "scripting/squirrel_error.hpp"
#include <sstream>

namespace Scripting
{

SquirrelError::SquirrelError(HSQUIRRELVM v, const std::string& message) throw()
{
  std::ostringstream msg;
  msg << "Squirrel error: " << message << " (";
  const char* lasterr;
  sq_getlasterror(v);
  if(sq_gettype(v, -1) != OT_STRING)
  {
    lasterr = "no error info";
  }
  else
  {
    sq_getstring(v, -1, &lasterr);
  }
  msg << lasterr << ")";
  sq_pop(v, 1);
  this->message = msg.str();
}

SquirrelError::~SquirrelError() throw()
{}

const char*
SquirrelError::what() const throw()
{
  return message.c_str();
}

}
