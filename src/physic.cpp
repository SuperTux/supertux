//  $Id$
// 
//  SuperTux
//  Copyright (C) 2004 Tobias Glaesser <tobi.web@gmx.de>
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

#include <stdio.h>

#include "scene.h"
#include "defines.h"
#include "physic.h"
#include "timer.h"
#include "world.h"
#include "level.h"

Physic::Physic()
    : ax(0), ay(0), vx(0), vy(0), gravity_enabled(true)
{
}

Physic::~Physic()
{
}

void
Physic::reset()
{
    ax = ay = vx = vy = 0;
    gravity_enabled = true;
}

void
Physic::set_velocity_x(float nvx)
{
  vx = nvx;
}

void
Physic::set_velocity_y(float nvy)
{
  vy = -nvy;
}

void
Physic::set_velocity(float nvx, float nvy)
{
  vx = nvx;
  vy = -nvy;
}

void Physic::inverse_velocity_x()
{
vx = -vx;
}

void Physic::inverse_velocity_y()
{
vy = -vy;
}

float
Physic::get_velocity_x()
{
    return vx;
}

float
Physic::get_velocity_y()
{
    return -vy;
}

void
Physic::set_acceleration_x(float nax)
{
  ax = nax;
}

void
Physic::set_acceleration_y(float nay)
{
  ay = -nay;
}

void
Physic::set_acceleration(float nax, float nay)
{
    ax = nax;
    ay = -nay;
}

float
Physic::get_acceleration_x()
{
    return ax;
}

float
Physic::get_acceleration_y()
{
    return -ay;
}

void
Physic::enable_gravity(bool enable_gravity)
{
  gravity_enabled = enable_gravity;
}

void
Physic::apply(float frame_ratio, float &x, float &y)
{
  float gravity = World::current()->get_level()->gravity;
  float grav;
  if(gravity_enabled)
    grav = gravity / 100.0;
  else
    grav = 0;

  x += vx * frame_ratio + ax * frame_ratio * frame_ratio;
  y += vy * frame_ratio + (ay + grav) * frame_ratio * frame_ratio;
  vx += ax * frame_ratio;
  vy += (ay + grav) * frame_ratio;
}
