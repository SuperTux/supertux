//  $Id$
//
//  SuperTux -  A Jump'n Run
//  Copyright (C) 2003 Tobias Glaesser <tobi.web@gmx.de>
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SUPERTUX_EXCEPTIONS_H
#define SUPERTUX_EXCEPTIONS_H

// Exceptions
#include <exception>
#include <string>

class SuperTuxException : public std::exception
{
  public:
    SuperTuxException(const char* _message, const char* _file = "", const unsigned int _line = 0)
      : message(_message), file(_file), line(_line) { };
    virtual ~SuperTuxException() throw() { };

    const char* what() const throw() { return message; };
    const char* what_file() const throw() { return file; };
    const unsigned int what_line() const throw() { return line; };
    
  private:
    const char* message;
    const char* file;
    const unsigned int line;
};

#endif /*SUPERTUX_EXCEPTIONS_H*/
