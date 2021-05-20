//  SuperTux - Sector scripting
//  Copyright (C) 2006 Wolfgang Becker <uafr@gmx.de>
//                2021 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_SCRIPTING_SECTOR_HPP
#define HEADER_SUPERTUX_SCRIPTING_SECTOR_HPP

#ifndef SCRIPTING_API
#include <string>
#include "scripting/game_object_manager.hpp"
class Sector;
#endif

namespace scripting {

class Sector final : public GameObjectManager
{
#ifndef SCRIPTING_API
private:
  ::Sector* m_parent;

public:
  Sector(::Sector* parent);

private:
  Sector(const Sector&) = delete;
  Sector& operator=(const Sector&) = delete;
#endif

public:
  void set_gravity(float gravity);
};

} // namespace scripting

#endif

/* EOF */
