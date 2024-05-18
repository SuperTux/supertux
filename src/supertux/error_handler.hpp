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

#ifndef HEADER_SUPERTUX_SUPERTUX_ERROR_HANDLER_HPP
#define HEADER_SUPERTUX_SUPERTUX_ERROR_HANDLER_HPP

#include <iostream>

class ErrorHandler final
{
public:
  static void set_handlers();

  static std::string get_stacktrace();
  static std::string get_system_info();

  static void error_dialog_crash(const std::string& stacktrace);
  static void error_dialog_exception(const std::string& exception = "");

  static void report_error(const std::string& details);

  [[ noreturn ]] static void handle_error(int sig);

  [[ noreturn ]] static void close_program();

private:
  static bool m_handing_error;

private:
  ErrorHandler() = delete;
  ~ErrorHandler() = delete;
  ErrorHandler(const ErrorHandler&) = delete;
  ErrorHandler& operator=(const ErrorHandler&) = delete;
};

#endif

/* EOF */
