//  $Id$
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef __GLUTIL_HPP__
#define __GLUTIL_HPP__

#include <sstream>
#include <stdexcept>
#include <GL/gl.h>

static inline void assert_gl(const char* message)
{
#ifdef DEBUG
  GLenum error = glGetError();
  if(error != GL_NO_ERROR) {
    std::ostringstream msg;
    msg << "OpenGLError while '" << message << "': ";
    switch(error) {
      case GL_INVALID_ENUM:
        msg << "INVALID_ENUM: An unacceptable value is specified for an "
               "enumerated argument.";
        break;
      case GL_INVALID_VALUE:
        msg << "INVALID_VALUE: A numeric argument is out of range.";
        break;
      case GL_INVALID_OPERATION:
        msg << "INVALID_OPERATION: The specified operation is not allowed "
               "in the current state.";
        break;
      case GL_STACK_OVERFLOW:
        msg << "STACK_OVERFLOW: This command would cause a stack overflow.";
        break;
      case GL_STACK_UNDERFLOW:
        msg << "STACK_UNDERFLOW: This command would cause a stack underflow.";
        break;
      case GL_OUT_OF_MEMORY:
        msg << "OUT_OF_MEMORY: There is not enough memory left to execute the "
               "command.";
        break;
      default:
        msg << "Unknown error (code " << error << ")";
    }
        
    throw std::runtime_error(msg.str());
  }
#endif
}

#endif

