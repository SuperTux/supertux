//
// C++ Interface: physic
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_PHYSIC_H
#define SUPERTUX_PHYSIC_H

#include "timer.h"

enum {
  PH_VT, /* Vertical throw.*/
  PH_HA  /* Horizontal acceleration. */
};

/* Physic type: */

typedef struct physic_type
  {
      int state;
      float start_vy;
      float start_vx;
      float acceleration;
      unsigned int start_time;
  }
physic_type;

/* global variables. */
extern float gravity;

void physic_init(physic_type* pphysic);
int physic_get_state(physic_type* pphysic);
void physic_set_state(physic_type* pphysic, int nstate);
void physic_set_start_vy(physic_type* pphysic, float start_vy);
void physic_set_start_vx(physic_type* pphysic, float start_vx);
void physic_set_acceleration(physic_type* pphysic, float acceleration);
int physic_is_set(physic_type* pphysic);
float physic_get_velocity(physic_type* pphysic);
float physic_get_max_distance(physic_type* pphysic);
unsigned int physic_get_max_time(physic_type* pphysic);
unsigned int physic_get_time_gone(physic_type* pphysic);

#endif /*SUPERTUX_PHYSIC_H*/
