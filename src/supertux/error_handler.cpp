//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#include "supertux/error_handler.hpp"

// execinfo.h as a built-in libc feature is exclusive to glibc as of 2020.
// On FreeBSD and musl systems, an external libexecinfo is available, but
// it has to be explicitly linked into the final executable.
// This is a *libc* feature, not a compiler one; furthermore, it's possible
// to verify its availability in CMakeLists.txt, if one is so inclined.

#include <csignal>
#include <sstream>
#include <string>

#include <SDL.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>


#pragma comment(lib, "DbgHelp.lib")
#elif defined(__unix__)
#include <execinfo.h>
#include <unistd.h>
#endif

bool ErrorHandler::m_handing_error = false;

void
ErrorHandler::set_handlers()
{
  signal(SIGSEGV, handle_error);
  signal(SIGABRT, handle_error);
}

std::string ErrorHandler::get_stacktrace()
{
#ifdef WIN32
  std::stringstream stacktrace;

  // Initialize symbols
  SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
  if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
  {
    return "";
  }

  // Get current stack frame
  void* stack[100];
  WORD frames = CaptureStackBackTrace(0, 100, stack, NULL);

  // Get symbols for each frame
  SYMBOL_INFO* symbol = static_cast<SYMBOL_INFO*>(std::calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1));
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  for (int i = 0; i < frames; i++)
  {
    SymFromAddr(GetCurrentProcess(), (DWORD64) stack[i], 0, symbol);
    stacktrace << symbol->Name << " - 0x" << std::hex << symbol->Address << "\n";
  }

  std::free(symbol);
  SymCleanup(GetCurrentProcess());

  return stacktrace.str();
#elif defined(__GLIBC__)
  void* array[128];
  size_t size;

  // Get void*'s for all entries on the stack.
  size = backtrace(array, 127);

  char** functions = backtrace_symbols(array, static_cast<int>(size));
  if (functions == nullptr)
    return "";

  std::stringstream stacktrace;

  for (size_t i = 0; i < size; i++)
    stacktrace << functions[i] << "\n";

  return stacktrace.str();
#endif
}

[[ noreturn ]] void
ErrorHandler::handle_error(int sig)
{
  if (m_handing_error)
  {
    // Error happened again while handling another segfault. Abort now.
    close_program();
  }
  else
  {
    m_handing_error = true;

    // Do not use external stuff (like log_fatal) to limit the risk of causing
    // another error, which would restart the handler again.
    fprintf(stderr, "\nError: signal %d:\n", sig);

    print_stack_trace();
    close_program();
  }
}

void
ErrorHandler::print_stack_trace()
{
  std::string stacktrace = get_stacktrace();
  std::stringstream stream;
  stream << "SuperTux has encountered an unrecoverable error!\n";
  if (!stacktrace.empty())
  {
    stream
      << "Send this error message (either by copying or screenshotting) and file a "
         "GitHub issue at https://github.com/SuperTux/supertux/issues/new.\n"
      << "Stacktrace:\n"
      << stacktrace;
  }
  else
  {
    stream << "Unable to get stacktrace.";
  }

  std::string msg = stream.str();

  std::cerr << msg << std::endl;

  SDL_ShowSimpleMessageBox(
    SDL_MESSAGEBOX_ERROR,
    "Error",
    msg.c_str(),
    nullptr
  );
}

[[ noreturn ]] void
ErrorHandler::close_program()
{
  exit(10);
}

/* EOF */
