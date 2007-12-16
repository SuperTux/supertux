//          Copyright Timothy Goya 2007.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNISON_VFS_STREAM_HPP
#define UNISON_VFS_STREAM_HPP

#include <iostream>

namespace Unison
{
   namespace VFS
   {
      class istream : public std::istream
      {
         public:
            istream(const std::string &filename);
            ~istream();
      };

      class ostream : public std::ostream
      {
         public:
            ostream(const std::string &filename);
            ~ostream();
      };
   }
}

#endif
