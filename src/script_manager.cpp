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
#include <sqstdio.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdsystem.h>
#include <sqstdmath.h>
#include <sqstdstring.h>

#include "timer.hpp"
#include "console.hpp"
#include "scripting/wrapper.hpp"
#include "scripting/wrapper_util.hpp"
#include "scripting/squirrel_error.hpp"
#include "physfs/physfs_stream.hpp"

using namespace Scripting;

ScriptManager* script_manager = 0;

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
{
  v = sq_open(1024);
  if(v == 0)
    throw std::runtime_error("Couldn't initialize squirrel vm");

  // register default error handlers
  sqstd_seterrorhandlers(v);
  // register squirrel libs
  sq_pushroottable(v);
  if(sqstd_register_bloblib(v) < 0)
    throw SquirrelError(v, "Couldn't register blob lib");
  if(sqstd_register_iolib(v) < 0)
    throw SquirrelError(v, "Couldn't register io lib");
  if(sqstd_register_systemlib(v) < 0)
    throw SquirrelError(v, "Couldn't register system lib");
  if(sqstd_register_mathlib(v) < 0)
    throw SquirrelError(v, "Couldn't register math lib");
  if(sqstd_register_stringlib(v) < 0)
    throw SquirrelError(v, "Couldn't register string lib");

  // register print function
  sq_setprintfunc(v, printfunc);
  
  // register supertux API
  register_supertux_wrapper(v);
}

ScriptManager::~ScriptManager()
{
  for(SquirrelVMs::iterator i = squirrel_vms.begin(); i != squirrel_vms.end(); ++i)
    sq_release(v, &(i->vm_obj));

  sq_close(v);
}

HSQUIRRELVM
ScriptManager::create_thread()
{
  HSQUIRRELVM vm = sq_newthread(v, 1024);
  if(vm == NULL)
    throw SquirrelError(v, "Couldn't create new VM");

  // retrieve reference to thread from stack and increase refcounter
  HSQOBJECT vm_obj;
  sq_resetobject(&vm_obj);
  if(SQ_FAILED(sq_getstackobj(v, -1, &vm_obj))) {
    throw SquirrelError(v, "Couldn't get coroutine vm from stack");
  }
  sq_addref(v, &vm_obj);
  sq_pop(v, 1);
  
  squirrel_vms.push_back(SquirrelVM(vm, vm_obj));

  return vm;
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
        sq_release(v, &squirrel_vm.vm_obj);
        i = squirrel_vms.erase(i);
        continue;
      }
    }
	
    if (vm_state != SQ_VMSTATE_SUSPENDED) {
      sq_release(v, &(squirrel_vm.vm_obj));
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
  for(SquirrelVMs::iterator i = squirrel_vms.begin(); i != squirrel_vms.end(); ++i) {
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
  for(SquirrelVMs::iterator i = squirrel_vms.begin(); i != squirrel_vms.end(); ++i) 
    {
      SquirrelVM& vm = *i;
      if(vm.waiting_for_events.type == event.type && vm.waiting_for_events.type != NO_EVENT)
        {
          vm.wakeup_time = game_time;
          break;
        }
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

