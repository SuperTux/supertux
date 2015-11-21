//  SuperTux - Worldmap Spawnpoint
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

#include "util/log.hpp"
#include "util/reader.hpp"
#include "worldmap/spawn_point.hpp"

namespace worldmap {

SpawnPoint::SpawnPoint(const ReaderMapping& slisp) :
  name(),
  pos(),
  auto_dir(D_NONE)
{
  pos.x = -1;
  pos.y = -1;
  auto iter = slisp.get_iter();
  while(iter.next()) {
    const std::string& token = iter.item();
    if(token == "name") {
      iter.get(name);
    } else if(token == "x") {
      iter.get(pos.x);
    } else if(token == "y") {
      iter.get(pos.y);
    } else if(token == "auto-dir") {
      std::string s = "";
      iter.get(s);
      auto_dir = string_to_direction(s);
    } else {
      log_warning << "unknown token '" << token << "' in SpawnPoint" << std::endl;
    }
  }

  if(name == "")
    throw std::runtime_error("No name specified for spawnpoint");
  if(pos.x < 0 || pos.y < 0)
    throw std::runtime_error("Invalid coordinates for spawnpoint");
}

}

/* EOF */
