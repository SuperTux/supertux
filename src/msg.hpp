//  $Id: debug.cpp 2650 2005-06-28 12:42:08Z sommer $
// 
//  SuperTux Debug Helper Functions
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef __SUPERTUX_MSG_H__
#define __SUPERTUX_MSG_H__

#include <iostream>
#include <stdio.h>

#include "console.hpp"

#ifdef DEBUG

inline std::ostream& msg_debug_f(const char* file, int line) {
  Console::output << "[DEBUG] " << file << " l." << line << ": ";
  return Console::output;
}

inline std::ostream& msg_info_f(const char* file, int line) {
  Console::output << "[INFO] " << file << " l." << line << ": ";
  return Console::output;
}

inline std::ostream& msg_warning_f(const char* file, int line) {
  Console::output << "[WARNING] " << file << " l." << line << ": ";
  return Console::output;
}

inline std::ostream& msg_fatal_f(const char* file, int line) {
  Console::output << "[FATAL] " << file << " l." << line << ": ";
  return Console::output;
}

#define msg_debug msg_debug_f(__FILE__, __LINE__)
#define msg_info msg_info_f(__FILE__, __LINE__)
#define msg_warning msg_warning_f(__FILE__, __LINE__)
#define msg_fatal msg_fatal_f(__FILE__, __LINE__)

#else

inline std::ostream& msg_debug_f(const char* file, int line) {
  return Console::output;
}

inline std::ostream& msg_info_f(const char* file, int line) {
  return Console::output;
}

inline std::ostream& msg_warning_f(const char* file, int line) {
  Console::output << "Warning: ";
  return Console::output;
}

inline std::ostream& msg_fatal_f(const char* file, int line) {
  Console::output << "Fatal: ";
  return Console::output;
}

#define msg_debug if (0) msg_debug_f(__FILE__, __LINE__)
#define msg_info msg_info_f(__FILE__, __LINE__)
#define msg_warning msg_warning_f(__FILE__, __LINE__)
#define msg_fatal msg_fatal_f(__FILE__, __LINE__)

#endif

#endif

