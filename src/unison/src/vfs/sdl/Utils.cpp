//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/vfs/sdl/Utils.hpp>
#include <unison/vfs/FileSystem.hpp>

#include <fstream>
#include <stdexcept>
#include <assert.h>
#include <physfs.h>
#include "SDL.h"

namespace
{
   int rwops_seek(SDL_RWops *context, int offset, int whence)
   {
      PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);
      int res = 0;
      switch(whence) {
          case SEEK_SET:
              res = PHYSFS_seek(file, offset);
              break;
          case SEEK_CUR:
              res = PHYSFS_seek(file, PHYSFS_tell(file) + offset);
              break;
          case SEEK_END:
              res = PHYSFS_seek(file, PHYSFS_fileLength(file) + offset);
              break;
          default:
              assert(0);
              break;
      }

      return (int) PHYSFS_tell(file);
   }

   int rwops_read(SDL_RWops *context, void *ptr, int size, int maxnum)
   {
      PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);

      int res = PHYSFS_read(file, ptr, size, maxnum);
      return res;
   }

   int rwops_write(SDL_RWops *context, const void *ptr, int size, int num)
   {
      PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);

      int res = PHYSFS_write(file, ptr, size, num);
      return res;
   }

   int rwops_close(SDL_RWops *context)
   {
      PHYSFS_File *file = static_cast<PHYSFS_File *>(context->hidden.unknown.data1);

      PHYSFS_close(file);
      delete context;

      return 0;
   }
}


namespace Unison
{
   namespace VFS
   {
      namespace SDL
      {
         SDL_RWops *Utils::open_physfs_in(const std::string &filename)
         {
            PHYSFS_File *file = PHYSFS_openRead((Unison::VFS::FileSystem::get().does_normalization() ? Unison::VFS::FileSystem::normalize(filename) : filename).c_str());
            if(!file)
            {
               throw std::runtime_error("Failed to open file '" + filename + "': " + std::string(PHYSFS_getLastError()));
            }
            SDL_RWops* ops = new SDL_RWops;
            ops->type = 0;
            ops->hidden.unknown.data1 = file;
            ops->seek = rwops_seek;
            ops->read = rwops_read;
            ops->write = 0; 
            ops->close = rwops_close;
            return ops;
         }

         SDL_RWops *Utils::open_physfs_out(const std::string &filename)
         {
            PHYSFS_File *file = PHYSFS_openWrite((Unison::VFS::FileSystem::get().does_normalization() ? Unison::VFS::FileSystem::normalize(filename) : filename).c_str());
            if(!file)
            {
               throw std::runtime_error("Failed to open file '" + filename + "': " + std::string(PHYSFS_getLastError()));
            }
            SDL_RWops* ops = new SDL_RWops;
            ops->type = 0;
            ops->hidden.unknown.data1 = file;
            ops->seek = rwops_seek;
            ops->read = 0;
            ops->write = rwops_write; 
            ops->close = rwops_close;
            return ops;
         }
      }
   }
}
