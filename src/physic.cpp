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
#include "defines.h"
#include "physic.h"

float gravity;

void physic_init(physic_type* pphysic)
{
  pphysic->state = -1;
  pphysic->start_time = 0;
  pphysic->start_vy = 0;
}

int physic_get_state(physic_type* pphysic)
{
  return pphysic->state;
}

void physic_set_state(physic_type* pphysic, int nstate)
{
  pphysic->state = nstate;
  pphysic->start_time = st_get_ticks();
}

void physic_set_start_vy(physic_type* pphysic, float start_vy)
{
  pphysic->start_vy = start_vy;
}

void physic_set_start_vx(physic_type* pphysic, float start_vx)
{
  pphysic->start_vx = start_vx;
}

void physic_set_acceleration(physic_type* pphysic, float acceleration)
{
  pphysic->acceleration = acceleration;
}


int physic_is_set(physic_type* pphysic)
{
  if(pphysic->state != -1)
    return YES;
  else
    return NO;
}

float physic_get_velocity(physic_type* pphysic)
{
  if(pphysic->state == PH_VT)
    return - (pphysic->start_vy - gravity* ((float)(st_get_ticks() - pphysic->start_time))/1000.);
  else if(pphysic->state == PH_HA)
    return - (pphysic->start_vx - pphysic->acceleration * ((float)(st_get_ticks() - pphysic->start_time))/1000.);
}

float physic_get_max_distance(physic_type* pphysic)
{
  return (pphysic->start_vy * pphysic->start_vy / 2.*gravity);
}

unsigned int physic_get_max_time(physic_type* pphysic)
{
  return (unsigned int)((pphysic->start_vy / gravity) * 1000);
}

unsigned int physic_get_time_gone(physic_type* pphysic)
{
  return st_get_ticks() - pphysic->start_time;
}


