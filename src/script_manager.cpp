//  $Id: main.cpp 3275 2006-04-09 00:32:34Z sommer $
// 
//  SuperTux
//  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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
#include <config.h>

#include "script_manager.hpp"

#include <stdarg.h>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include "timer.hpp"
#include "console.hpp"
#include "scripting/wrapper.hpp"
#include "scripting/wrapper_util.hpp"
#include "scripting/squirrel_error.hpp"
#include "physfs/physfs_stream.hpp"

using namespace Scripting;

ScriptManager* ScriptManager::instance = NULL;

static void printfunc(HSQUIRRELVM, const char* str, ...)
{
  char buf[4096];
  va_list arglist; 
  va_start(arglist, str); 
  vsprintf(buf, str, arglist);
  Console::output << (const char*) buf << std::flush;
  va_end(arglist); 
}

ScriptManager::ScriptManager()
  : parent(NULL)
{
  vm = sq_open(64);
  if(vm == 0)
    throw std::runtime_error("Couldn't initialize squirrel vm");
  sq_setforeignptr(vm, (SQUserPointer) this);

  // register squirrel libs
  sq_pushroottable(vm);
  if(sqstd_register_bloblib(vm) < 0)
    throw SquirrelError(vm, "Couldn't register blob lib");
  if(sqstd_register_mathlib(vm) < 0)
    throw SquirrelError(vm, "Couldn't register math lib");
  if(sqstd_register_stringlib(vm) < 0)
    throw SquirrelError(vm, "Couldn't register string lib");
  // register supertux API
  register_supertux_wrapper(vm);
  sq_pop(vm, 1);

  // register print function
  sq_setprintfunc(vm, printfunc);
  // register default error handlers
  sqstd_seterrorhandlers(vm); 
}

ScriptManager::ScriptManager(ScriptManager* parent)
{
  this->parent = parent;
  vm = parent->vm;
  parent->childs.push_back(this);
}

ScriptManager::~ScriptManager()
{
  for(SquirrelVMs::iterator i = squirrel_vms.begin();
      i != squirrel_vms.end(); ++i)
    sq_release(vm, &(i->vm_obj));

  if(parent != NULL) {
    parent->childs.erase(
        std::remove(parent->childs.begin(), parent->childs.end(), this),
        parent->childs.end());
  } else {
    sq_close(vm);
  }
}

HSQUIRRELVM
ScriptManager::create_thread(bool leave_thread_on_stack)
{
  HSQUIRRELVM new_vm = sq_newthread(vm, 64);
  if(new_vm == NULL)
    throw SquirrelError(vm, "Couldn't create new VM");
  sq_setforeignptr(new_vm, (SQUserPointer) this);

  // retrieve reference to thread from stack and increase refcounter
  HSQOBJECT vm_obj;
  sq_resetobject(&vm_obj);
  if(SQ_FAILED(sq_getstackobj(vm, -1, &vm_obj))) {
    throw SquirrelError(vm, "Couldn't get coroutine vm from stack");
  }
  sq_addref(vm, &vm_obj);

  if(!leave_thread_on_stack)
    sq_pop(vm, 1);
  
  squirrel_vms.push_back(SquirrelVM(new_vm, vm_obj));

  return new_vm;
}

void
ScriptManager::update()
{
  for(SquirrelVMs::iterator i = squirrel_vms.begin(); i != squirrel_vms.end(); ) {
    SquirrelVM& squirrel_vm = *i;
    int vm_state = sq_getvmstate(squirrel_vm.vm);
    
    if(vm_state == SQ_VMSTATE_SUSPENDED && squirrel_vm.wakeup_time > 0 && game_time >= squirrel_vm.wakeup_time) {
      squirrel_vm.waiting_for_events = WakeupData(NO_EVENT);
      try {
        if(SQ_FAILED(sq_wakeupvm(squirrel_vm.vm, false, false))) {
          throw SquirrelError(squirrel_vm.vm, "Couldn't resume script");
        }
      } catch(std::exception& e) {
        std::cerr << "Problem executing script: " << e.what() << "\n";
        sq_release(vm, &squirrel_vm.vm_obj);
        i = squirrel_vms.erase(i);
        continue;
      }
    }
	
    if (vm_state != SQ_VMSTATE_SUSPENDED) {
      sq_release(vm, &(squirrel_vm.vm_obj));
      i = squirrel_vms.erase(i);
    } else {
      ++i;
    }
  }
}

void
ScriptManager::set_wakeup_event(HSQUIRRELVM vm, WakeupData event, float timeout)
{
  assert(event.type >= 0 && event.type < WAKEUP_EVENT_COUNT);
  // find the VM in the list and update it
  for(SquirrelVMs::iterator i = squirrel_vms.begin();
      i != squirrel_vms.end(); ++i) {
    SquirrelVM& squirrel_vm = *i;
    if(squirrel_vm.vm == vm) 
      {
        squirrel_vm.waiting_for_events = event;

        if(timeout < 0) {
          squirrel_vm.wakeup_time = -1;
        } else {
          squirrel_vm.wakeup_time = game_time + timeout;
        }
        return;
      }
  }
}

void
ScriptManager::fire_wakeup_event(WakeupData  event)
{
  assert(event.type >= 0 && event.type < WAKEUP_EVENT_COUNT);
  for(SquirrelVMs::iterator i = squirrel_vms.begin();
      i != squirrel_vms.end(); ++i) {
    SquirrelVM& vm = *i;
    if(vm.waiting_for_events.type == event.type
        && vm.waiting_for_events.type != NO_EVENT) {
      vm.wakeup_time = game_time;
      break;
    }
  }

  for(std::vector<ScriptManager*>::iterator i = childs.begin();
      i != childs.end(); ++i) {
    ScriptManager* child = *i;
    child->fire_wakeup_event(event);
  }
}

void
ScriptManager::set_wakeup_event(HSQUIRRELVM vm, WakeupEvent event, float timeout)
{
  set_wakeup_event(vm, WakeupData(event), timeout);
}

void
ScriptManager::fire_wakeup_event(WakeupEvent event)
{
  fire_wakeup_event(WakeupData(event));
}

ScriptManager::SquirrelVM::SquirrelVM(HSQUIRRELVM arg_vm, HSQOBJECT arg_obj)
  : vm(arg_vm), vm_obj(arg_obj)
{
  waiting_for_events = WakeupData(NO_EVENT);
  wakeup_time        = 0;
}

