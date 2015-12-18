//  SuperTux Debug Helper Functions
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
//  Copyright (C) 2010 Florian Forster <supertux at octo.it>
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

#include "util/log.hpp"

#include <iostream>

#include "math/rectf.hpp"
#include "supertux/console.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/globals.hpp"

LogLevel g_log_level = LOG_WARNING;

static std::ostream& get_logging_instance (bool use_console_buffer = true)
{
  if (ConsoleBuffer::current() && use_console_buffer)
    return (ConsoleBuffer::output);
  else
    return (std::cerr);
}

static std::ostream& log_generic_f (const char *prefix, const char* file, int line, bool use_console_buffer = true)
{
  get_logging_instance (use_console_buffer) << prefix << " " << file << ":" << line << " ";
  return (get_logging_instance (use_console_buffer));
}

std::ostream& log_debug_f(const char* file, int line, bool use_console_buffer = true)
{
  return (log_generic_f ("[DEBUG]", file, line, use_console_buffer));
}

std::ostream& log_info_f(const char* file, int line)
{
  return (log_generic_f ("[INFO]", file, line));
}

std::ostream& log_warning_f(const char* file, int line)
{
  if(g_config && g_config->developer_mode &&
     Console::current() && !Console::current()->hasFocus()) {
    Console::current()->open();
  }
  return (log_generic_f ("[WARNING]", file, line));
}

std::ostream& log_fatal_f(const char* file, int line)
{
  if(g_config && g_config->developer_mode &&
     Console::current() && !Console::current()->hasFocus()) {
    Console::current()->open();
  }
  return (log_generic_f ("[FATAL]", file, line));
}

std::ostream& operator<<(std::ostream& out, const Vector& vector)
{
  out << '[' << vector.x << ',' << vector.y << ']';
  return out;
}

std::ostream& operator<<(std::ostream& out, const Rectf& rect)
{
  out << "[" << rect.get_left() << "," << rect.get_top() << "   "
      << rect.get_right() << "," << rect.get_bottom() << "]";
  return out;
}

/* Callbacks used by tinygettext */
void log_info_callback(const std::string& str)
{
    log_info << "\r\n[TINYGETTEXT] " << str << std::endl;
}

void log_warning_callback(const std::string& str)
{
    log_debug << "\r\n[TINYGETTEXT] " << str << std::endl;
}

void log_error_callback(const std::string& str)
{
    log_warning << "\r\n[TINYGETTEXT] " << str << std::endl;
}

/* EOF */
