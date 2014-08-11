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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "util/string_util.hpp"

int main(int argc, char** argv)
{
  std::vector<std::string> lines;
  
  // read files from stdin or files
  std::string line;
  if (argc > 1)
  {
    for(int i = 1; i < argc; ++i)
    {
      std::ifstream in(argv[i]);
      while(std::getline(in, line))
      {
        lines.push_back(line);
      }
    }
  }
  else
  {
    while(std::getline(std::cin, line))
    {
      lines.push_back(line);
    }
  }
  
  // sort lines
  std::sort(lines.begin(), lines.end(), StringUtil::numeric_less);

  // output the sorted text
  for(std::vector<std::string>::iterator i = lines.begin(); i != lines.end(); ++i)
  {
    std::cout << *i << std::endl;
  }

  return 0;
}

/* EOF */
