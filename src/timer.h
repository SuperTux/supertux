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

extern unsigned int st_pause_ticks, st_pause_count;

unsigned int st_get_ticks(void);
void st_pause_ticks_init(void);
void st_pause_ticks_start(void);
void st_pause_ticks_stop(void);

class timer_type
{
 public:
  unsigned int period;
  unsigned int time;
  unsigned int (*get_ticks) (void);  

 public:
  void init(bool st_ticks);
  void start(unsigned int period);
  void stop();

  /*======================================================================
    int timer_check(timer_type* ptimer);
    
    param : pointer to a timer which needs to be checked
    return: NO  = the timer is not started
    or it is over
    YES = otherwise
    ======================================================================*/
  int check();
  int started();

  /*======================================================================
    int timer_get_left(timer_type* ptimer);

    param : pointer to a timer that you want to get the time left
    return: the time left (in millisecond)
    note  : the returned value can be negative
    ======================================================================*/
  int get_left();

  int  get_gone();
  void fwrite(FILE* fi);
  void fread(FILE* fi);
};

#endif /*SUPERTUX_TIMER_H*/

/* Local Variables: */
/* mode:c++ */
/* End */
