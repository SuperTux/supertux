//
// C Interface: timer
//
// Description: 
//
//
// Author: Tobias Glaesser <tobi.web@gmx.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef SUPERTUX_TIMER_H
#define SUPERTUX_TIMER_H

/* Timer type */
typedef struct timer_type
  {
   unsigned int period;
   unsigned int time;
  }
timer_type;

void timer_init(timer_type* ptimer);
void timer_start(timer_type* ptimer, unsigned int period);
void timer_stop(timer_type* ptimer);
int timer_check(timer_type* ptimer);
int timer_started(timer_type* ptimer);
int timer_get_left(timer_type* ptimer);
int timer_get_gone(timer_type* ptimer);

#endif /*SUPERTUX_TIMER_H*/

