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

#include "math/size.hpp"

int main()
{
  Size size(800, 600);

  std::cout << size << std::endl;
  std::cout << size * 2 << std::endl;
  std::cout << 2 * size << std::endl;
  std::cout << size / 2 << std::endl;
  std::cout << size + size << std::endl;
  size *= 2;
  std::cout << size << std::endl;
  size /= 2;
  std::cout << size << std::endl;
  size += size;
  std::cout << size << std::endl;

  return 0;
}

/* EOF */
