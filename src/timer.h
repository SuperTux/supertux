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

#ifndef SUPERTUX_TIMER_H
#define SUPERTUX_TIMER_H

extern unsigned int st_pause_ticks, st_pause_count;

unsigned int st_get_ticks(void);
void st_pause_ticks_init(void);
void st_pause_ticks_start(void);
void st_pause_ticks_stop(void);
bool st_pause_ticks_started(void);

class Timer
{
 public:
  unsigned int period;
  unsigned int time;
  unsigned int (*get_ticks) (void);  

 public:
  Timer();
  
  void init(bool st_ticks);
  void start(unsigned int period);
  void stop();

  /*======================================================================
    return: NO  = the timer is not started
    or it is over
    YES = otherwise
    ======================================================================*/
  int check();
  int started();

  /*======================================================================
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
/* End: */
