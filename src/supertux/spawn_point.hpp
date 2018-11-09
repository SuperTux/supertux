//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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

#ifndef HEADER_SUPERTUX_SUPERTUX_SPAWN_POINT_HPP
#define HEADER_SUPERTUX_SUPERTUX_SPAWN_POINT_HPP

#include <string>

#include "math/vector.hpp"

class ReaderMapping;
class Writer;

class SpawnPoint final
{
public:
  SpawnPoint();
  SpawnPoint(const std::string& name, const Vector& pos);
  SpawnPoint(const ReaderMapping& lisp);

  void save(Writer& writer);

  std::string get_name() const { return m_name; }
  Vector get_pos() const { return m_pos; }
  void set_pos(const Vector& pos) { m_pos = pos; }

private:
  std::string m_name;
  Vector m_pos;

private:
  SpawnPoint(const SpawnPoint&) = delete;
  SpawnPoint& operator=(const SpawnPoint&) = delete;
};

#endif

/* EOF */
