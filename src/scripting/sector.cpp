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
#include "supertux/game_object_factory.hpp"
#include "supertux/moving_object.hpp"
#include "supertux/sector.hpp"
#include "util/log.hpp"
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

void
Sector::add_object(const std::string& class_name, const std::string& name,
                   int posX, int posY, const std::string& direction,
                   const std::string& data)
{
  if(name.empty())
  {
    log_fatal << "Object name cannot be empty" << std::endl;
    return; 
  } 

  if(m_parent->get_object_by_name<GameObject>(name) != nullptr)
  {
    log_fatal << "Object with name " << name << " already exists in sector" << std::endl;
    return; 
  }

  std::unique_ptr<GameObject> obj =
    GameObjectFactory::instance().create(class_name, Vector(posX, posY), string_to_dir(direction), data);

  if(dynamic_cast<MovingObject*>(obj.get()) == nullptr)
  {
    log_fatal << "Only MovingObject instances can be created via scripting" << std::endl;
    return; 
  }

  obj->set_name(name);
  m_parent->add_object(std::move(obj));
}

} // namespace scripting

/* EOF */
