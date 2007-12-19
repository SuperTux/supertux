//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <unison/vfs/stream.hpp>
#include <unison/vfs/FileSystem.hpp>

#include <streambuf>
#include <stdexcept>
#include <assert.h>
#include <physfs.h>

// FIXME: make streambufs more complete
namespace
{
   class iphysfsfilebuf : public std::streambuf
   {
      public:
         iphysfsfilebuf(const std::string &filename)
         {
            file = PHYSFS_openRead((Unison::VFS::FileSystem::get().does_normalization() ? Unison::VFS::FileSystem::normalize(filename) : filename).c_str());
            if(!file)
            {
               throw std::runtime_error("Failed to open file '" + filename + "': " + std::string(PHYSFS_getLastError()));
            }
         }

         ~iphysfsfilebuf()
         {
            PHYSFS_close(file);
         }
      protected:
         int underflow()
         {
            if(PHYSFS_eof(file))
            {
               return traits_type::eof();
            }
            PHYSFS_sint64 bytes = PHYSFS_read(file, buffer, 1, sizeof(buffer));
            if(bytes <= 0)
            {
               return traits_type::eof();
            }
            setg(buffer, buffer, buffer + bytes);
            return buffer[0];
         }

         pos_type seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
         {
            off_type nsp = off;
            PHYSFS_sint64 pos = PHYSFS_tell(file);
            switch(way)
            {
               case std::ios_base::beg:
                  break;
               case std::ios_base::cur:
                  if(off == 0)
                  {
                     return pos - (egptr() - gptr());
                  }
                  nsp += pos - (egptr() - gptr());
                  break;
               case std::ios_base::end:
                  nsp += PHYSFS_fileLength(file);
                  break;
               default:
                  assert(0);
                  break;
            }
            return seekpos(nsp, which);
         }

         pos_type seekpos(pos_type sp, std::ios_base::openmode /*which*/)
         {
            if(PHYSFS_seek(file, sp) == 0)
            {
               return -1;
            }
            setg(buffer, buffer, buffer);
            return sp;
         }
      private:
         PHYSFS_File *file;
         char buffer[1024];
   };

   class ophysfsfilebuf : public std::streambuf
   {
      public:
         ophysfsfilebuf(const std::string &filename)
         {
            file = PHYSFS_openWrite((Unison::VFS::FileSystem::get().does_normalization() ? Unison::VFS::FileSystem::normalize(filename) : filename).c_str());
            if(!file)
            {
               throw std::runtime_error("Failed to open file '" + filename + "': " + std::string(PHYSFS_getLastError()));
            }
            setp(buffer, buffer + sizeof(buffer));
         }

         ~ophysfsfilebuf()
         {
            sync();
            PHYSFS_close(file);
         }
      protected:
         int sync()
         {
            return overflow(traits_type::eof());
         }

         int overflow(int c)
         {
            char ch = static_cast<char>(c);
            size_t size = pptr() - pbase();
            if(size == 0)
            {
               return 0;
            }
            PHYSFS_sint64 bytes = PHYSFS_write(file, pbase(), 1, size);
            if(bytes <= 0)
            {
               return traits_type::eof();
            }
            if(c != traits_type::eof())
            {
               PHYSFS_sint64 bytes = PHYSFS_write(file, &ch, 1, 1);
               if(bytes <= 0)
               {
                  return traits_type::eof();
               }
            }
            setp(buffer, buffer + bytes);
            return 0;
         }
      private:
         PHYSFS_File *file;
         char buffer[1024];
   };
}

namespace Unison
{
   namespace VFS
   {
      istream::istream(const std::string &filename) :
         std::istream(new iphysfsfilebuf(filename))
      {
      }

      istream::~istream()
      {
         delete rdbuf();
      }

      ostream::ostream(const std::string &filename) :
         std::ostream(new ophysfsfilebuf(filename))
      {
      }

      ostream::~ostream()
      {
         delete rdbuf();
      }
   }
}
