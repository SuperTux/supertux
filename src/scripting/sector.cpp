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

#include <physfs.h>

#include "object/background.hpp"
#include "object/gradient.hpp"
#include "supertux/sector.hpp"

namespace {

Background* get_background_image(::Sector* parent)
{
  auto background = parent->get_background_image();
  if(background == NULL)
  {
    log_info << "No background image found" << std::endl;
    return NULL;
  }
  return background;
}

}

namespace scripting {

Sector::Sector(::Sector* parent) :
  m_parent(parent)
{
}

Sector::~Sector()
{
}

void
Sector::set_ambient_light(float red, float green, float blue)
{
  m_parent->set_ambient_light(red, green, blue);
}

float
Sector::get_ambient_red() const
{
  return m_parent->get_ambient_red();
}

float
Sector::get_ambient_green() const
{
  return m_parent->get_ambient_green();
}

float
Sector::get_ambient_blue() const
{
  return m_parent->get_ambient_blue();
}

void
Sector::set_background_image(const std::string& image)
{
  auto background_image = get_background_image(m_parent);
  if(!background_image)
    return;

  if(image.empty())
  {
    log_info << "No filename / path for background image specified" << std::endl;
    return;
  }

  const std::string& default_dir = "images/background/";
  bool path_valid = true;

  if(!PHYSFS_exists(image.c_str()))
    path_valid = false;

  background_image->set_image(path_valid ? image : default_dir + image);
}

void
Sector::set_background_images(const std::string& top_image, const std::string& middle_image,
                              const std::string& bottom_image)
{
  auto background_image = get_background_image(m_parent);
  if(!background_image)
    return;

  if(top_image.empty() || middle_image.empty() || bottom_image.empty())
  {
    log_info << "No filename / path for background image specified" << std::endl;
    return;
  }

  const std::string& default_dir = "images/background/";
  bool top_image_valid = true, middle_image_valid = true, bottom_image_valid = true;

  if(!PHYSFS_exists(top_image.c_str()))
    top_image_valid = false;

  if(!PHYSFS_exists(middle_image.c_str()))
    middle_image_valid = false;

  if(!PHYSFS_exists(bottom_image.c_str()))
    bottom_image_valid = false;

  background_image->set_images(top_image_valid ? top_image : default_dir + top_image,
                               middle_image_valid ? middle_image : default_dir + middle_image,
                               bottom_image_valid ? bottom_image : default_dir + bottom_image);
}

void
Sector::set_background_speed(float speed)
{
  auto background_image = get_background_image(m_parent);
  if(!background_image)
    return;

  background_image->set_image(background_image->get_image(), speed);
}

void
Sector::set_gravity(float gravity)
{
  m_parent->set_gravity(gravity);
}

} // namespace scripting

/* EOF */
