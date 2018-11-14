//  SuperTux
//  Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
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

#include "object/ambient_light.hpp"
#include "object/music_object.hpp"
#include "supertux/sector.hpp"
#include "video/color.hpp"

namespace scripting {

Sector::Sector(::Sector* parent) :
  m_parent(parent)
{
}

void
Sector::fade_to_ambient_light(float red, float green, float blue, float fadetime)
{
  if (auto* ambient_light = m_parent->get_object_by_type<AmbientLight>()) {
    ambient_light->fade_to_ambient_light(red, green, blue, fadetime);
  }
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  if (auto* ambient_light = m_parent->get_object_by_type<AmbientLight>()) {
    ambient_light->set_ambient_light(Color(red, green, blue));
  }
}

float
Sector::get_ambient_red() const
{
  if (auto* ambient_light = m_parent->get_object_by_type<AmbientLight>()) {
    return ambient_light->get_ambient_light().red;
  } else {
    return 1.0f;
  }
}

float
Sector::get_ambient_green() const
{
  if (auto* ambient_light = m_parent->get_object_by_type<AmbientLight>()) {
    return ambient_light->get_ambient_light().green;
  } else {
    return 1.0f;
  }
}

float
Sector::get_ambient_blue() const
{
  if (auto* ambient_light = m_parent->get_object_by_type<AmbientLight>()) {
    return ambient_light->get_ambient_light().blue;
  } else {
    return 1.0f;
  }
}

void
Sector::set_gravity(float gravity)
{
  m_parent->set_gravity(gravity);
}

void
Sector::set_music(const std::string& filename)
{
  if (auto* music = m_parent->get_object_by_type<MusicObject>()) {
    music->set_music(filename);
  }
}

} // namespace scripting

/* EOF */
