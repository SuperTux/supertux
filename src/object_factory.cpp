//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2004 Ricardo Cruz <rick2@aeiou.pt>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#include <config.h>

#include <sstream>
#include <stdexcept>

#include "lisp/lisp.h"
#include "lisp/parser.h"
#include "object_factory.h"

Factories* object_factories = 0;

GameObject* create_object(const std::string& name, const lisp::Lisp& reader)
{
  Factories::iterator i = object_factories->find(name);
  if(i == object_factories->end()) {
    std::stringstream msg;
    msg << "No factory for object '" << name << "' found.";
    throw std::runtime_error(msg.str());
  }

  return i->second->create_object(reader);
}

GameObject* create_object(const std::string& name, const Vector& pos)
{
  std::stringstream lisptext;
  lisptext << "(" << name
           << " (x " << pos.x << ")"
           << " (y " << pos.y << "))";
  
  lisp::Parser parser;
  std::auto_ptr<lisp::Lisp> lisp (parser.parse(lisptext));
  return create_object(name, *lisp);
}
