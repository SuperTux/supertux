//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef _SDL_GL_H
#define _SDL_GL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;

#define GL_UNSIGNED_BYTE 0x1401
#define GL_QUADS 0x0007
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_BLEND 0x0BE2
#define GL_ZERO 0x0
#define GL_ONE 0x1
#define GL_SRC_COLOR 0x0300
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_COLOR 0x0306
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_CLAMP 0x2900
#define GL_LINEAR 0x2601
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_PROC(ret, func, params) extern ret (*func) params;
#include "SDL_glfuncs.h"
#undef GL_PROC

int glLoad(const char *path);

#ifdef __cplusplus
}
#endif

#endif
