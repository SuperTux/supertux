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

#ifndef HEADER_SUPERTUX_SQUIRREL_SQUIRREL_SCHEDULER_HPP
#define HEADER_SUPERTUX_SQUIRREL_SQUIRREL_SCHEDULER_HPP

#include <vector>

#include <squirrel.h>

class SquirrelVM;

/** This class keeps a list of squirrel threads that are scheduled for a certain
    time. (the typical result of a wait() command in a squirrel script) */
class SquirrelScheduler final
{
public:
  SquirrelScheduler(SquirrelVM& vm);

  /** time must be absolute time, not relative updates, i.e. g_game_time */
  void update(float time);
  void schedule_thread(HSQUIRRELVM vm, float time, bool skippable);

private:
  struct ScheduleEntry {
    /// weak reference to the squirrel vm object
    HSQOBJECT thread_ref;
    /// time when the thread should be woken up
    float wakeup_time;
    // true if calling force_wake_up should wake this entry up
    bool skippable;

    bool operator<(const ScheduleEntry& other) const
    {
      // we need the smallest value on top
      return wakeup_time > other.wakeup_time;
    }
  };

private:
  SquirrelVM& m_vm;

  typedef std::vector<ScheduleEntry> ScheduleHeap;
  ScheduleHeap schedule;

private:
  SquirrelScheduler(const SquirrelScheduler&) = delete;
  SquirrelScheduler& operator=(const SquirrelScheduler&) = delete;
};

#endif

/* EOF */
