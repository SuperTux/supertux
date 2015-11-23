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

#include <stdexcept>

#include "supertux/spawn_point.hpp"
#include "util/log.hpp"
#include "util/reader.hpp"

SpawnPoint::SpawnPoint() :
  name(),
  pos()
{}

SpawnPoint::SpawnPoint(const SpawnPoint& other) :
  name(other.name),
  pos(other.pos)
{}

SpawnPoint::SpawnPoint(const ReaderMapping& slisp) :
  name(),
  pos()
{
  pos.x = -1;
  pos.y = -1;

  slisp.get("name", name);
  slisp.get("x", pos.x);
  slisp.get("y", pos.y);

  if(name.empty())
    log_warning << "No name specified for spawnpoint. Ignoring." << std::endl;
  if(pos.x < 0 || pos.y < 0)
    log_warning << "Invalid coordinates specified for spawnpoint. Ignoring." << std::endl;
}

/* EOF */
