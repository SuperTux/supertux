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

#ifndef HEADER_SUPERTUX_WORLDMAP_SPAWN_POINT_HPP
#define HEADER_SUPERTUX_WORLDMAP_SPAWN_POINT_HPP

#include <string>

#include "util/reader_fwd.hpp"
#include "math/vector.hpp"
#include "supertux/game_object.hpp"
#include "worldmap/direction.hpp"

namespace WorldMapNS {

class SpawnPoint
{
public:
  SpawnPoint(const Reader& lisp);

  std::string name;
  Vector pos;
  Direction auto_dir; /**< automatically start walking in this direction */
};

}

#endif

/* EOF */
