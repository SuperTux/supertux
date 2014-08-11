//  SuperTux
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmail.com>
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

#include <iostream>
#include <errno.h>
#include <string.h>

#include "addon/md5.hpp"

int main(int argc, char** argv)
{
  for(int i = 1; i < argc; ++i)
  {
    std::ifstream in(argv[i], std::ios::binary);
    if (!in)
    {
      std::cerr << argv[0] << ": " << argv[i] << ": " << strerror(errno) << std::endl;
    }
    else
    {
      MD5 md5(in);
      std::cout << md5.hex_digest() << "  " << argv[i] << std::endl;
    }
  }  
  return 0;
}

/* EOF */
