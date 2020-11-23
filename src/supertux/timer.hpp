//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_SUPERTUX_TIMER_HPP
#define HEADER_SUPERTUX_SUPERTUX_TIMER_HPP

#include "supertux/globals.hpp"

class ReaderMapping;
class Writer;

/** Simple timer designed to be used in the update functions of
    objects */
class Timer final
{
public:
  Timer();

  /** start the timer with the given period (in seconds). If
      cyclic=true then the timer will be reset after each period. Set
      period to zero if you want to disable the timer. */
  void start(float period, bool cyclic = false);

  /** returns true if a period (or more) passed since start call or last
      successful check */
  bool check();

  /** stop the timer */
  void stop() { start(0); }

  /** returns the period of the timer or 0 if it isn't started */
  float get_period() const { return m_period; }
  float get_timeleft() const{ return m_period - (g_game_time - m_cycle_start); }
  float get_timegone() const { return g_game_time - m_cycle_start; }
  bool started() const { return m_period != 0 && get_timeleft() > 0; }

  void backup(Writer& writer);
  void restore(const ReaderMapping& reader);

private:
  float m_period;
  float m_cycle_start;
  bool m_cyclic;

private:
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
};

#endif

/* EOF */
