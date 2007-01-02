//  $Id$
//
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
#ifndef SUPERTUX_PHYSIC_H
#define SUPERTUX_PHYSIC_H

#include "math/vector.hpp"

/// Physics engine.
/** This is a very simplistic physics engine handling accelerated and constant
  * movement along with gravity.
  */
class Physic
{
public:
  Physic();

  Vector get_movement(float elapsed_time);

  /// horizontal and vertical acceleration
  float ax, ay;
  /// horizontal and vertical velocity
  float vx, vy;
  /// should we respect gravity in our calculations?
  bool gravity_enabled;
  /// current gravity to apply to object, if enabled
  float gravity;
};

#endif
