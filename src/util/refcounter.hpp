//  Windstille - A Jump'n Shoot Game
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_UTIL_REFCOUNTER_HPP
#define HEADER_SUPERTUX_UTIL_REFCOUNTER_HPP

#include <assert.h>

/**
 * A base class that provides reference counting facilities
 */
class RefCounter
{
public:
  RefCounter()
    : refcount(0)
  { }

  /** increases reference count */
  void ref()
  {
    refcount++;
  }
  /** decreases reference count. Destroys the object if the reference count
   * reaches 0
   */
  void unref()
  {
    refcount--;
    if(refcount <= 0) {
      delete this;
    }
  }

protected:
  virtual ~RefCounter()
  {
    assert(refcount == 0);
  }

private:
  int refcount;
};

#endif

/* EOF */
