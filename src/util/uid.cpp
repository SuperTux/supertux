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

#include "util/uid.hpp"

#include <ostream>

std::ostream& operator<<(std::ostream& os, const UID& uid)
{
  return os << uid.m_value;
}

bool UID::operator==(const UID& other) const {
  //std::cout << "Comparing " << m_value << " and " << other.m_value << std::endl;
  if ((m_value & 0xff000000u) && (other.m_value & 0xff000000u))
  {
    //std::cout << "Raw: " << (m_value == other.m_value) << std::endl;
    return m_value == other.m_value;
  }
  else
  {
    //std::cout << "Comparing " << m_value << "(" << (m_value & 0x00ffffffu) << ") and " << other.m_value << "(" << (other.m_value & 0x00ffffffu) << ")" << std::endl;
    //std::cout << "Smart: " << ((m_value & 0x00ffffffu) == (other.m_value & 0x00ffffffu)) << std::endl;
    return (m_value & 0x00ffffffu) == (other.m_value & 0x00ffffffu);
  }
}

namespace std {

size_t hash<UID>::operator()(const UID& uid) const
{
  return static_cast<size_t>(uid.m_value);
}

} // namespace std

/* EOF */
