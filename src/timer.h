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
   unsigned int (*get_ticks) (void);  
  }
timer_type;

extern unsigned int st_pause_ticks, st_pause_count;

unsigned int st_get_ticks(void);
void st_pause_ticks_init(void);
void st_pause_ticks_start(void);
void st_pause_ticks_stop(void);
void timer_init(timer_type* ptimer, int st_ticks);
void timer_start(timer_type* ptimer, unsigned int period);
void timer_stop(timer_type* ptimer);
/*======================================================================
     int timer_check(timer_type* ptimer);
    
     param : pointer to a timer which needs to be checked
     return: NO  = the timer is not started
                   or it is over
             YES = otherwise
======================================================================*/
int timer_check(timer_type* ptimer);
int timer_started(timer_type* ptimer);
/*======================================================================
     int timer_get_left(timer_type* ptimer);

     param : pointer to a timer that you want to get the time left
     return: the time left (in millisecond)
     note  : the returned value can be negative
======================================================================*/
int timer_get_left(timer_type* ptimer);
int timer_get_gone(timer_type* ptimer);
int timer_fwrite(timer_type* ptimer, FILE* fi);
int timer_fread(timer_type* ptimer, FILE* fi);

#endif /*SUPERTUX_TIMER_H*/

