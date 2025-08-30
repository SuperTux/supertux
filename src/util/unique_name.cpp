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

#include "util/unique_name.hpp"

#include <sstream>
#include <stdint.h>
#include <time.h>

std::string make_unique_name(const std::string& prefix, void* ptr)
{
  std::ostringstream out;
  out << "gen" << time(nullptr) << reinterpret_cast<uintptr_t>(ptr);
  return out.str();
}
