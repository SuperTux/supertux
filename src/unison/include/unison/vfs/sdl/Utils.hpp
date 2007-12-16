//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VFS_SDL_UTILS_HPP
#define UNISON_VFS_SDL_UTILS_HPP

#include <string>

#include "SDL.h"

namespace Unison
{
   namespace VFS
   {
      namespace SDL
      {
         struct Utils
         {
            static SDL_RWops *open_physfs_in(const std::string &filename);
         };
      }
   }
}

#endif
