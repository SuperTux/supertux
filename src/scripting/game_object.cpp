//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
//  Copyright (C) 2023 Vankata453
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

#include "scripting/game_object.hpp"

#include "supertux/sector.hpp"
#include "worldmap/worldmap.hpp"

namespace scripting {

::GameObjectManager& get_game_object_manager()
{
  if (::worldmap::WorldMap::current())
    return *::worldmap::WorldMap::current();
  if (::Sector::current())
    return ::Sector::get();

  throw std::runtime_error("Unable to perform scripting GameObject action: Neither sector, nor worldmap active.");
}

} // namespace scripting

/* EOF */
