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

#include "scripting/game_object_manager.hpp"

#include "object/ambient_light.hpp"
#include "object/music_object.hpp"
#include "supertux/game_object_manager.hpp"
#include "video/color.hpp"

namespace scripting {

GameObjectManager::GameObjectManager(::GameObjectManager* parent) :
  m_gom_parent(parent)
{
}

void
GameObjectManager::fade_to_ambient_light(float red, float green, float blue, float fadetime)
{
  auto& ambient_light = m_gom_parent->get_singleton_by_type<AmbientLight>();
  ambient_light.fade_to_ambient_light(red, green, blue, fadetime);
}

void
GameObjectManager::set_ambient_light(float red, float green, float blue)
{
  auto& ambient_light = m_gom_parent->get_singleton_by_type<AmbientLight>();
  ambient_light.set_ambient_light(Color(red, green, blue));
}

float
GameObjectManager::get_ambient_red() const
{
  auto& ambient_light = m_gom_parent->get_singleton_by_type<AmbientLight>();
  return ambient_light.get_ambient_light().red;
}

float
GameObjectManager::get_ambient_green() const
{
  auto& ambient_light = m_gom_parent->get_singleton_by_type<AmbientLight>();
  return ambient_light.get_ambient_light().green;
}

float
GameObjectManager::get_ambient_blue() const
{
  auto& ambient_light = m_gom_parent->get_singleton_by_type<AmbientLight>();
  return ambient_light.get_ambient_light().blue;
}

void
GameObjectManager::set_music(const std::string& filename)
{
  auto& music = m_gom_parent->get_singleton_by_type<MusicObject>();
  music.set_music(filename);
}

} // namespace scripting

/* EOF */
