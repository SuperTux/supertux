//  SuperTux
//  Copyright (C) 2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_WORLDMAP_HPP
#define HEADER_SUPERTUX_SCRIPTING_WORLDMAP_HPP


#ifndef SCRIPTING_API
#include <string>
#include "scripting/game_object_manager.hpp"
namespace worldmap {
class WorldMap;
}
#endif

namespace scripting {

class WorldMap final : public GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::worldmap::WorldMap* m_parent;

public:
  WorldMap(::worldmap::WorldMap* parent);

private:
  WorldMap(const WorldMap&) = delete;
  WorldMap& operator=(const WorldMap&) = delete;
#endif

public:
  float get_tux_x();
  float get_tux_y();
};

} // namespace scripting

#endif

/* EOF */
