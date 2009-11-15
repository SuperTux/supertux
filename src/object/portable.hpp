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

#ifndef HEADER_SUPERTUX_OBJECT_PORTABLE_HPP
#define HEADER_SUPERTUX_OBJECT_PORTABLE_HPP

#include "supertux/moving_object.hpp"
#include "supertux/direction.hpp"
#include "util/refcounter.hpp"

/**
 * An object that inherits from this object is considered "portable" and can
 * be carried around by the player.
 * The object has to additionally set the PORTABLE flag (this allows to
 * make the object only temporarily portable by resetting the flag)
 */
class Portable
{
public:
  virtual ~Portable()
  { }

  /**
   * called each frame when the object has been grabbed.
   */
  virtual void grab(MovingObject& object, const Vector& pos, Direction dir) = 0;

  virtual void ungrab(MovingObject& , Direction )
  {}

  virtual bool is_portable() const
  {
    return true;
  }
};

#endif
