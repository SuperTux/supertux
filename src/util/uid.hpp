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

#ifndef HEADER_SUPERTUX_UTIL_UID_HPP
#define HEADER_SUPERTUX_UTIL_UID_HPP

#include <assert.h>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <stdint.h>
#include <string>

class UID;

namespace std {

template<>
struct hash<UID>
{
  size_t operator()(const UID& uid) const;
};

} // namespace std {

class UID
{
  friend class UIDGenerator;
  friend std::ostream& operator<<(std::ostream& os, const UID& uid);
  friend size_t std::hash<UID>::operator()(const UID&) const;

public:
  using Magic = uint8_t;

public:
  UID() : m_value(0) {}
  UID(const UID& other) = default;
  UID& operator=(const UID& other) = default;

  explicit UID(uint32_t value) :
    m_value(value)
  {
    assert(m_value != 0);
  }

  inline operator bool() const {
    return m_value != 0;
  }

  inline bool operator<(const UID& other) const {
    return m_value < other.m_value;
  }
  bool operator==(const UID& other) const;
  inline bool operator!=(const UID& other) const {
    return !operator==(other);
  }

  inline Magic get_magic() const { return static_cast<Magic>((m_value & 0xffff0000u) >> 16); }

  inline std::string to_string() const { return "uid" + std::to_string(m_value); }
  inline std::string key_string() const { return "uidkey" + std::to_string(m_value & 0x00ffffffu); }
  inline static UID from_string(std::string s) { return UID(static_cast<uint32_t>(std::stoul(s.substr(3)))); }
  inline static UID from_key_string(std::string s) { return UID(static_cast<uint32_t>(std::stoul(s.substr(6)))); }

private:
  uint32_t m_value;
};

std::ostream& operator<<(std::ostream& os, const UID& uid);

#endif

/* EOF */
