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

#ifndef HEADER_SUPERTUX_LISP_LIST_ITERATOR_HPP
#define HEADER_SUPERTUX_LISP_LIST_ITERATOR_HPP

#include "lisp/lisp.hpp"

namespace lisp {

/**
 * Small and a bit hacky helper class that helps parsing lisp lists where all
 * entries are lists again themselves
 */
class ListIterator
{
public:
  ListIterator(const lisp::Lisp* cur);

  const std::string& item() const
  { return current_item; }
  const lisp::Lisp* lisp() const
  { return current_lisp; }
  const lisp::Lisp* value() const
  { return current_lisp->get_car(); }
  bool next();

private:
  std::string current_item;
  const lisp::Lisp* current_lisp;
  const lisp::Lisp* cur;

private:
  ListIterator(const ListIterator&);
  ListIterator& operator=(const ListIterator&);
};

}

#endif

/* EOF */
