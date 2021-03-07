//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "scripting/sector.hpp"

#include "math/easing.hpp"
#include "object/ambient_light.hpp"
#include "object/music_object.hpp"
#include "supertux/sector.hpp"
#include "video/color.hpp"

namespace scripting {

Sector::Sector(::Sector* parent) :
  GameObjectManager(parent),
  m_parent(parent)
{
}

void
Sector::set_gravity(float gravity)
{
  m_parent->set_gravity(gravity);
}

} // namespace scripting

/* EOF */
