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

#include <version.h>

#include "util/file_system.hpp"

#if (defined(__unix__) || defined(__APPLE__)) && !(defined(__EMSCRIPTEN__))
#define UNIX
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
//#include <VersionHelpers.h>

#pragma comment(lib, "DbgHelp.lib")
#elif defined(UNIX)
#include <sys/utsname.h>
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

std::string
ErrorHandler::get_stacktrace()
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
#elif defined(UNIX)
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

  std::free(functions);

  return stacktrace.str();
#else
  return "";
#endif
}

std::string
ErrorHandler::get_system_info()
{
#ifdef WIN32

  std::stringstream info;

/*
  // This method reports Windows 8 on my
  // Windows 10 PC. Disabled.
  if (IsWindows10OrGreater())
    info << "Windows 10/11";
  else if (IsWindows8Point1OrGreater())
    info << "Windows 8.1";
  else if (IsWindows8OrGreater())
    info << "Windows 8";
  else if (IsWindows7OrGreater())
    info << "Windows 7";
  else if (IsWindowsVistaOrGreater())
    info << "Windows Vista";
  else if (IsWindowsXPOrGreater())
    info << "Windows XP";
  else
    info << "Windows";

  info << " ";
*/

  info << "Windows ";


  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);

  switch (sysinfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
      info << "x64";
      break;
    case PROCESSOR_ARCHITECTURE_ARM:
      info << "ARM";
      break;
    case PROCESSOR_ARCHITECTURE_ARM64:
      info << "ARM64";
      break;
    case PROCESSOR_ARCHITECTURE_IA64:
      info << "Intel Itanium-based";
      break;
    case PROCESSOR_ARCHITECTURE_INTEL:
      info << "x86";
      break;
    default:
      break;
  }

  return info.str();

#elif defined(UNIX)
  struct utsname uts;
  uname(&uts);
  std::stringstream info;
  info << uts.sysname << " "
       << uts.release << " "
       << uts.version << " "
       << uts.machine;
  return info.str();
#else
  return "";
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
    std::cerr << "\nError: signal " << sig << ":\n";

    error_dialog_crash(get_stacktrace());
    close_program();
  }
}

void
ErrorHandler::error_dialog_crash(const std::string& stacktrace)
{
  char msg[] = "SuperTux has encountered an unrecoverable error!";

  std::cerr << msg << "\n" << stacktrace << std::endl;

  SDL_MessageBoxButtonData btns[] = {
    {
      0, // flags
      0, // buttonid
      "Report" // text
    },
    {
      0, // flags
      1, // buttonid
      "Details" // text
    },
    {
      SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, // flags
      2, // buttonid
      "OK" // text
    }
  };

  SDL_MessageBoxData data = {
    SDL_MESSAGEBOX_ERROR, // flags
    nullptr, // window
    "Error", // title
    msg, // message
    SDL_arraysize(btns), // numbuttons
    btns, // buttons
    nullptr // colorscheme
  };

  int resultbtn;
  SDL_ShowMessageBox(&data, &resultbtn);

  switch (resultbtn)
  {
    case 0:
      report_error(stacktrace);
      break;

    case 1:
    {
      SDL_MessageBoxButtonData detailsbtns[] = {
        {
          0, // flags
          0, // buttonid
          "Report" // text
        },
        {
          SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, // flags
          1, // buttonid
          "OK" // text
        }
      };

      data = {
        SDL_MESSAGEBOX_ERROR, // flags
        nullptr, // window
        "Error details", // title
        stacktrace.c_str(), // message
        SDL_arraysize(detailsbtns), // numbuttons
        detailsbtns, // buttons
        nullptr // colorscheme
      };

      SDL_ShowMessageBox(&data, &resultbtn);

      if (resultbtn == 0)
        report_error(stacktrace);

      break;
    }

    default:
      break;
  }
}

void
ErrorHandler::error_dialog_exception(const std::string& exception)
{
  std::stringstream stream;

  stream << "SuperTux has encountered a fatal exception!";

  if (!exception.empty())
  {
    stream << "\n\n" << exception;
  }

  std::string msg = stream.str();

  SDL_MessageBoxButtonData btns[] = {
#ifdef WIN32
    {
      0, // flags
      0, // buttonid
      "Open log" // text
    },
#endif
    {
      SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, // flags
      1, // buttonid
      "OK" // text
    }
  };

  SDL_MessageBoxData data = {
    SDL_MESSAGEBOX_ERROR, // flags
    nullptr, // window
    "Error", // title
    msg.c_str(), // message
    SDL_arraysize(btns), // numbuttons
    btns, // buttons
    nullptr // colorscheme
  };

  int resultbtn;
  SDL_ShowMessageBox(&data, &resultbtn);

#ifdef WIN32
  if (resultbtn == 0)
  {
    // Repurpose the stream.
    stream.str("");

    stream << SDL_GetPrefPath("SuperTux", "supertux2")
           << "/console.err";
    FileSystem::open_path(stream.str());
  }
#endif
}

void
ErrorHandler::report_error(const std::string& details)
{
  std::stringstream url;

  // cppcheck-suppress unknownMacro
  url << "https://github.com/supertux/supertux/issues/new"
         "?template=crash.yml"
         "&labels=type:crash,status:needs-confirmation"
         "&supertux-version=" << FileSystem::escape_url(PACKAGE_VERSION) <<
         "&system-info=" << FileSystem::escape_url(get_system_info()) <<
         "&debug-stacktrace=" << FileSystem::escape_url(details);

  FileSystem::open_url(url.str());
}

[[ noreturn ]] void
ErrorHandler::close_program()
{
  _Exit(10);
}

/* EOF */
