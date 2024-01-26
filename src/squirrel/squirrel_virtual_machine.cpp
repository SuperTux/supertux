//  SuperTux
//  Copyright (C) 2014 Ingo Ruhnke <grumbel@gmail.com>
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

#include "squirrel/squirrel_virtual_machine.hpp"

#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

#include "physfs/ifile_stream.hpp"
#include "scripting/wrapper.hpp"
#include "squirrel/squirrel_error.hpp"
#include "squirrel/squirrel_thread_queue.hpp"
#include "squirrel/squirrel_scheduler.hpp"
#include "squirrel_util.hpp"
#include "supertux/console.hpp"
#include "supertux/globals.hpp"
#include "util/log.hpp"

#ifdef ENABLE_SQDBG
#  include "../../external/squirrel/sqdbg/sqrdbg.h"
namespace {
HSQREMOTEDBG debugger = nullptr;
} // namespace
#endif

namespace {

#ifdef __clang__
__attribute__((__format__ (__printf__, 2, 0)))
#endif
void printfunc(HSQUIRRELVM, const char* fmt, ...)
{
  char buf[4096];
  const char separator[] = "\n";
  va_list arglist;
  va_start(arglist, fmt);
  vsnprintf(buf, sizeof(buf), fmt, arglist);
  const char* ptr = strtok(buf, separator);
  while (ptr != nullptr)
  {
    ConsoleBuffer::output << "[SCRIPTING] " << ptr << std::endl;
    ptr = strtok(nullptr, separator);
  }
  va_end(arglist);
}

} // namespace

SquirrelVirtualMachine::SquirrelVirtualMachine(bool enable_debugger) :
  m_vm(),
  m_screenswitch_queue(),
  m_scheduler()
{
  sq_setsharedforeignptr(m_vm.get_vm(), this);

  m_screenswitch_queue = std::make_unique<SquirrelThreadQueue>(m_vm);
  m_scheduler = std::make_unique<SquirrelScheduler>(m_vm);

  if (enable_debugger) {
#ifdef ENABLE_SQDBG
    sq_enabledebuginfo(m_vm.get_vm(), SQTrue);
    debugger = sq_rdbg_init(m_vm.get_vm(), 1234, SQFalse);
    if (debugger == nullptr)
      throw SquirrelError(m_vm.get_vm(), "Couldn't initialize squirrel debugger");

    sq_enabledebuginfo(m_vm.get_vm(), SQTrue);
    log_info << "Waiting for debug client..." << std::endl;
    if (SQ_FAILED(sq_rdbg_waitforconnections(debugger)))
      throw SquirrelError(m_vm.get_vm(), "Waiting for debug clients failed");
    log_info << "debug client connected." << std::endl;
#endif
  }

  sq_pushroottable(m_vm.get_vm());
  if (SQ_FAILED(sqstd_register_bloblib(m_vm.get_vm())))
    throw SquirrelError(m_vm.get_vm(), "Couldn't register blob lib");
  if (SQ_FAILED(sqstd_register_mathlib(m_vm.get_vm())))
    throw SquirrelError(m_vm.get_vm(), "Couldn't register math lib");
  if (SQ_FAILED(sqstd_register_stringlib(m_vm.get_vm())))
    throw SquirrelError(m_vm.get_vm(), "Couldn't register string lib");

  // remove rand and srand calls from sqstdmath, we'll provide our own
  m_vm.delete_table_entry("srand");
  m_vm.delete_table_entry("rand");

  // register supertux API
  scripting::register_supertux_wrapper(m_vm.get_vm());

  sq_pop(m_vm.get_vm(), 1);

  // register print function
  sq_setprintfunc(m_vm.get_vm(), printfunc, printfunc);
  // register default error handlers
  sqstd_seterrorhandlers(m_vm.get_vm());

  // try to load default script
  try {
    std::string filename = "scripts/default.nut";
    IFileStream stream(filename);
    compile_and_run(m_vm.get_vm(), stream, filename);
  } catch(std::exception& e) {
    log_warning << "Couldn't load default.nut: " << e.what() << std::endl;
  }
}

SquirrelVirtualMachine::~SquirrelVirtualMachine()
{
#ifdef ENABLE_SQDBG
  if (debugger != nullptr) {
    sq_rdbg_shutdown(debugger);
    debugger = nullptr;
  }
#endif
}

void
SquirrelVirtualMachine::update(float dt_sec)
{
  update_debugger();
  m_scheduler->update(g_game_time);
}

void
SquirrelVirtualMachine::update_debugger()
{
#ifdef ENABLE_SQDBG
  if (debugger != nullptr)
    sq_rdbg_update(debugger);
#endif
}

void
SquirrelVirtualMachine::wait_for_seconds(HSQUIRRELVM vm, float seconds)
{
  m_scheduler->schedule_thread(vm, g_game_time + seconds, false);
}

void
SquirrelVirtualMachine::skippable_wait_for_seconds(HSQUIRRELVM vm, float seconds)
{
  m_scheduler->schedule_thread(vm, g_game_time + seconds, true);
}

void
SquirrelVirtualMachine::wait_for_screenswitch(HSQUIRRELVM vm)
{
  m_screenswitch_queue->add(vm);
}

void
SquirrelVirtualMachine::wakeup_screenswitch()
{
  m_screenswitch_queue->wakeup();
}

/* EOF */
