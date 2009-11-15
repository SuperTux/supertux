//  SuperTux Debug Helper Functions
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include <config.h>

#include "math/rect.hpp"
#include "math/vector.hpp"
#include "supertux/console.hpp"
#include "util/log.hpp"

#ifdef DEBUG

std::ostream& log_debug_f(const char* file, int line) 
{
  Console::output << "[DEBUG] " << file << ":" << line << " ";
  return Console::output;
}

 std::ostream& log_info_f(const char* file, int line) 
{
  Console::output << "[INFO] " << file << ":" << line << " ";
  return Console::output;
}

 std::ostream& log_warning_f(const char* file, int line) 
{
  Console::output << "[WARNING] " << file << ":" << line << " ";
  return Console::output;
}

 std::ostream& log_fatal_f(const char* file, int line) 
{
  Console::output << "[FATAL] " << file << ":" << line << " ";
  return Console::output;
}

#else

std::ostream& log_fatal_f() 
{
  Console::output << "Fatal: ";
  return Console::output;
}

#endif

std::ostream& operator<<(std::ostream& out, const Vector& vector)
{
  out << '[' << vector.x << ',' << vector.y << ']';
  return out;
}

std::ostream& operator<<(std::ostream& out, const Rect& rect)
{
  out << "[" << rect.get_left() << "," << rect.get_top() << "   "
             << rect.get_right() << "," << rect.get_bottom() << "]";
  return out;
}

/* EOF */
