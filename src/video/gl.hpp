//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_VIDEO_GL_HPP
#define HEADER_SUPERTUX_VIDEO_GL_HPP

#include <config.h>

#ifdef HAVE_OPENGL

#if defined(USE_OPENGLES2)
#  include <SDL_opengles2.h>
#elif defined(USE_OPENGLES1)
#  include <SDL_opengles.h>
#else
#  ifdef USE_GLBINDING
#    include <glbinding/gl/gl.h>
#    include <glbinding/gl/bitfield.h>
#  else
#    include <GL/glew.h>
#    define GL_NONE_BIT 0
#  endif
#endif

#ifdef USE_OPENGLES1
#  define glOrtho glOrthof
#endif

#ifdef USE_OPENGLES2
// These are required for OpenGL3.3Core, but not availabel en GLES2,
// simple no-op replacement looks prettier than #ifdef
inline void glGenVertexArrays(GLsizei n, GLuint *arrays) {}
inline void glDeleteVertexArrays(GLsizei n, GLuint *arrays) {}
inline void glBindVertexArray(GLuint vao) {}
#endif

#ifdef USE_GLBINDING
using namespace gl;
#endif

#else

// These are used by SDL code when OpenGL is completely disabled
#define GLenum int
#define GLint int
#define GL_SRC_ALPHA 0
#define GL_ONE_MINUS_SRC_ALPHA 1
#define GL_RGBA 2
#define GL_ONE 3
#define GL_ZERO 4
#define GL_DST_COLOR 5
#define GL_LINEAR 6
#define GL_NEAREST 7
#define GL_CLAMP_TO_EDGE 8
#define GL_MIRRORED_REPEAT 9
#define GL_REPEAT 10

#endif

#endif

/* EOF */
