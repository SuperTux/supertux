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

#include "squirrel/squirrel_thread_queue.hpp"

#include "squirrel/squirrel_virtual_machine.hpp"
#include "squirrel/squirrel_util.hpp"
#include "util/log.hpp"

SquirrelThreadQueue::SquirrelThreadQueue(SquirrelVM& vm) :
  m_vm(vm),
  m_threads()
{
}

void
SquirrelThreadQueue::add(HSQUIRRELVM vm)
{
  // create a weakref to the VM
  sq_pushthread(m_vm.get_vm(), vm);
  sq_weakref(m_vm.get_vm(), -1);

  HSQOBJECT object;
  if (SQ_FAILED(sq_getstackobj(m_vm.get_vm(), -1, &object))) {
    sq_pop(m_vm.get_vm(), 2);
    throw SquirrelError(m_vm.get_vm(), "Couldn't get thread weakref from vm");
  }
  sq_addref(m_vm.get_vm(), &object);
  m_threads.push_back(object);

  sq_pop(m_vm.get_vm(), 2);
}

void
SquirrelThreadQueue::wakeup()
{
  SquirrelObjectList threads = std::move(m_threads);
  m_threads.clear();

  for (HSQOBJECT& object : threads)
  {
    sq_pushobject(m_vm.get_vm(), object);
    sq_getweakrefval(m_vm.get_vm(), -1);

    HSQUIRRELVM scheduled_vm;
    if (sq_gettype(m_vm.get_vm(), -1) == OT_THREAD &&
       SQ_SUCCEEDED(sq_getthread(m_vm.get_vm(), -1, &scheduled_vm)))
    {
      if (SQ_FAILED(sq_wakeupvm(scheduled_vm, SQFalse, SQFalse, SQTrue, SQFalse))) {
        log_warning << "Couldn't wakeup scheduled squirrel VM" << std::endl;
      }
    }

    sq_release(m_vm.get_vm(), &object);
    sq_pop(m_vm.get_vm(), 1);
  }
}

/* EOF */
