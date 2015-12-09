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

#include "scripting/scripting.hpp"

#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <stdarg.h>
#include <stdio.h>

#include "physfs/ifile_stream.hpp"
#include "scripting/squirrel_error.hpp"
#include "scripting/wrapper.hpp"
#include "squirrel_util.hpp"
#include "supertux/console.hpp"
#include "util/log.hpp"

#ifdef ENABLE_SQDBG
#  include "../../external/squirrel/sqdbg/sqrdbg.h"
namespace {
HSQREMOTEDBG debugger = NULL;
} // namespace
#endif

namespace {

#ifdef __clang__
__attribute__((__format__ (__printf__, 2, 0)))
#endif
void printfunc(HSQUIRRELVM, const char* fmt, ...)
{
  char buf[4096];
  va_list arglist;
  va_start(arglist, fmt);
  vsnprintf(buf, sizeof(buf), fmt, arglist);
  ConsoleBuffer::output << "[SQUIRREL] " << (const char*) buf << std::flush;
  va_end(arglist);
}

} // namespace

namespace scripting {

HSQUIRRELVM global_vm = NULL;

Scripting::Scripting(bool enable_debugger)
{
  global_vm = sq_open(64);
  if(global_vm == NULL)
    throw std::runtime_error("Couldn't initialize squirrel vm");

  if(enable_debugger) {
#ifdef ENABLE_SQDBG
    sq_enabledebuginfo(global_vm, SQTrue);
    debugger = sq_rdbg_init(global_vm, 1234, SQFalse);
    if(debugger == NULL)
      throw SquirrelError(global_vm, "Couldn't initialize squirrel debugger");

    sq_enabledebuginfo(global_vm, SQTrue);
    log_info << "Waiting for debug client..." << std::endl;
    if(SQ_FAILED(sq_rdbg_waitforconnections(debugger)))
      throw SquirrelError(global_vm, "Waiting for debug clients failed");
    log_info << "debug client connected." << std::endl;
#endif
  }

  sq_pushroottable(global_vm);
  if(SQ_FAILED(sqstd_register_bloblib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register blob lib");
  if(SQ_FAILED(sqstd_register_mathlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register math lib");
  if(SQ_FAILED(sqstd_register_stringlib(global_vm)))
    throw SquirrelError(global_vm, "Couldn't register string lib");

  // remove rand and srand calls from sqstdmath, we'll provide our own
  sq_pushstring(global_vm, "srand", -1);
  sq_deleteslot(global_vm, -2, SQFalse);
  sq_pushstring(global_vm, "rand", -1);
  sq_deleteslot(global_vm, -2, SQFalse);

  // register supertux API
  register_supertux_wrapper(global_vm);

  sq_pop(global_vm, 1);

  // register print function
  sq_setprintfunc(global_vm, printfunc, printfunc);
  // register default error handlers
  sqstd_seterrorhandlers(global_vm);

  // try to load default script
  try {
    std::string filename = "scripts/default.nut";
    IFileStream stream(filename);
    scripting::compile_and_run(global_vm, stream, filename);
  } catch(std::exception& e) {
    log_warning << "Couldn't load default.nut: " << e.what() << std::endl;
  }
}

Scripting::~Scripting()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL) {
    sq_rdbg_shutdown(debugger);
    debugger = NULL;
  }
#endif

  if (global_vm)
    sq_close(global_vm);

  global_vm = NULL;
}

void
Scripting::update_debugger()
{
#ifdef ENABLE_SQDBG
  if(debugger != NULL)
    sq_rdbg_update(debugger);
#endif
}

} // namespace scripting

/* EOF */
