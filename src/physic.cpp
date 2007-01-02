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
#include <config.h>

#include "physic.hpp"

Physic::Physic()
    : ax(0), ay(0), vx(0), vy(0), gravity_enabled(true), gravity(1000)
{
}

Vector
Physic::get_movement(float elapsed_time)
{
  float grav = gravity_enabled ? gravity : 0;

  Vector result(
      vx * elapsed_time + ax * elapsed_time * elapsed_time,
      vy * elapsed_time + (ay + grav) * elapsed_time * elapsed_time
  );
  vx += ax * elapsed_time;
  vy += (ay + grav) * elapsed_time;

  return result;
}
