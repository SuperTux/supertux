//
// C Implementation: physic
//
// Description:
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
