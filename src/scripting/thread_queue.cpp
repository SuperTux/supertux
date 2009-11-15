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

#include "scripting/thread_queue.hpp"

#include "scripting/squirrel_util.hpp"
#include "util/log.hpp"

namespace Scripting
{

ThreadQueue::ThreadQueue()
{
}

ThreadQueue::~ThreadQueue()
{
}

void
ThreadQueue::add(HSQUIRRELVM vm)
{
  // create a weakref to the VM
  HSQOBJECT vm_obj = vm_to_object(vm);
  sq_pushobject(global_vm, vm_obj);
  sq_weakref(global_vm, -1);

  HSQOBJECT object;
  if(SQ_FAILED(sq_getstackobj(global_vm, -1, &object))) {
    sq_pop(global_vm, 2);
    throw SquirrelError(global_vm, "Couldn't get thread weakref from vm");
  }
  sq_addref(global_vm, &object);
  threads.push_back(object);

  sq_pop(global_vm, 2);
}

void
ThreadQueue::wakeup()
{
  // we traverse the list in reverse orders and use indices. This should be
  // robust for scripts that add new entries to the list while we're traversing
  // it
  size_t i = threads.size() - 1;
  size_t end = (size_t) 0 - 1;
  size_t size_begin = threads.size();
  while(i != end) {
    HSQOBJECT object = threads[i];

    sq_pushobject(global_vm, object);
    sq_getweakrefval(global_vm, -1);

    HSQUIRRELVM scheduled_vm;
    if(sq_gettype(global_vm, -1) == OT_THREAD &&
       SQ_SUCCEEDED(sq_getthread(global_vm, -1, &scheduled_vm))) {
      if(SQ_FAILED(sq_wakeupvm(scheduled_vm, SQFalse, SQFalse, SQTrue, SQFalse))) {
        log_warning << "Couldn't wakeup scheduled squirrel VM" << std::endl;
      }
    }

    sq_release(global_vm, &object);
    sq_pop(global_vm, 1);
    i--;
  }

  threads.erase(threads.begin(), threads.begin() + size_begin);
}

}

/* EOF */
