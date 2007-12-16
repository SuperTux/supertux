//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "SDL.h"
#include "SDL_gl.h"

#define GL_PROC(ret, func, params) ret (*func) params;
#include "SDL_glfuncs.h"
#undef GL_PROC

int glLoad(const char *path)
{
   int ret = SDL_GL_LoadLibrary(path);
   if(ret)
   {
      return ret;
   }
#define GL_PROC(ret, func, params) func = SDL_GL_GetProcAddress(#func);
#include "SDL_glfuncs.h"
#undef GL_PROC
   return 0;
}
