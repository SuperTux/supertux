//  $Id$
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

namespace {

inline std::ostream& log_debug_f(const char* file, int line) {
  Console::output << "[DEBUG] " << file << ":" << line << " ";
  return Console::output;
}

inline std::ostream& log_info_f(const char* file, int line) {
  Console::output << "[INFO] " << file << ":" << line << " ";
  return Console::output;
}

inline std::ostream& log_warning_f(const char* file, int line) {
  Console::output << "[WARNING] " << file << ":" << line << " ";
  return Console::output;
}

inline std::ostream& log_fatal_f(const char* file, int line) {
  Console::output << "[FATAL] " << file << ":" << line << " ";
  return Console::output;
}

}

#define log_debug log_debug_f(__FILE__, __LINE__)
#define log_info log_info_f(__FILE__, __LINE__)
#define log_warning log_warning_f(__FILE__, __LINE__)
#define log_fatal log_fatal_f(__FILE__, __LINE__)

#else

namespace {

inline std::ostream& log_fatal_f() {
  Console::output << "Fatal: ";
  return Console::output;
}

}

#define log_debug if (0) std::cerr
#define log_info std::cout
#define log_warning std::cerr
#define log_fatal log_fatal_f()

#endif

class Vector;
std::ostream& operator<< (std::ostream& str, const Vector& vector);
class Rect;
std::ostream& operator<< (std::ostream& str, const Rect& rect);

#endif
