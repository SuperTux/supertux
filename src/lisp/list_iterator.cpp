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

#include "lisp/list_iterator.hpp"
#include <stdexcept>

namespace lisp {

ListIterator::ListIterator(const lisp::Lisp* newlisp) :
  current_item(),
  current_lisp(0),
  cur(newlisp)
{
}

bool
ListIterator::next()
{
  if(cur == 0)
    return false;

  const lisp::Lisp* child = cur->get_car();
  if(!child)
    throw std::runtime_error("child is 0 in list entry");
  if(child->get_type() != lisp::Lisp::TYPE_CONS)
    throw std::runtime_error("Expected CONS");
  const lisp::Lisp* name = child->get_car();
  if(!name || (
       name->get_type() != lisp::Lisp::TYPE_SYMBOL
       && name->get_type() != lisp::Lisp::TYPE_STRING))
    throw std::runtime_error("Expected symbol");
  name->get(current_item);
  current_lisp = child->get_cdr();

  cur = cur->get_cdr();
  return true;
}

}

/* EOF */
