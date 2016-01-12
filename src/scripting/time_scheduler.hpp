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

#ifndef HEADER_SUPERTUX_SCRIPTING_TIME_SCHEDULER_HPP
#define HEADER_SUPERTUX_SCRIPTING_TIME_SCHEDULER_HPP

#include <vector>

namespace scripting {

/**
 * This class keeps a list of squirrel threads that are scheduled for a certain
 * time. (the typical result of a wait() command in a squirrel script)
 */
class TimeScheduler
{
public:
  TimeScheduler();
  ~TimeScheduler();

  void update(float time);
  void schedule_thread(HSQUIRRELVM vm, float time);

  static TimeScheduler* instance;

private:
  struct ScheduleEntry {
    /// weak reference to the squirrel vm object
    HSQOBJECT thread_ref;
    /// time when the thread should be woken up
    float wakeup_time;

    bool operator<(const ScheduleEntry& other) const
    {
      // we need the smallest value on top
      return wakeup_time > other.wakeup_time;
    }
  };

  typedef std::vector<ScheduleEntry> ScheduleHeap;
  ScheduleHeap schedule;
};

}

#endif

/* EOF */
