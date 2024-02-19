//  SuperTux
//  Copyright (C) 2016 Hume2 <teratux.mail@gmail.com>
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

#include "scripting/background.hpp"

#include <physfs.h>

#include "object/background.hpp"

namespace scripting {

void
Background::set_image(const std::string& image)
{
  SCRIPT_GUARD_VOID;

  if (image.empty())
  {
    log_info << "No filename / path for background image specified" << std::endl;
    return;
  }

  const std::string& default_dir = "images/background/";
  bool path_valid = true;

  if (!PHYSFS_exists(image.c_str()))
    path_valid = false;

  object.set_image(path_valid ? image : default_dir + image);
}

void
Background::set_images(const std::string& top_image, const std::string& middle_image,
                       const std::string& bottom_image)
{
  SCRIPT_GUARD_VOID;

  if (top_image.empty() || middle_image.empty() || bottom_image.empty())
  {
    log_info << "No filename / path for background image specified" << std::endl;
    return;
  }

  const std::string& default_dir = "images/background/";
  bool top_image_valid = true, middle_image_valid = true, bottom_image_valid = true;

  if (!PHYSFS_exists(top_image.c_str()))
    top_image_valid = false;

  if (!PHYSFS_exists(middle_image.c_str()))
    middle_image_valid = false;

  if (!PHYSFS_exists(bottom_image.c_str()))
    bottom_image_valid = false;

  object.set_images(top_image_valid ? top_image : default_dir + top_image,
                    middle_image_valid ? middle_image : default_dir + middle_image,
                    bottom_image_valid ? bottom_image : default_dir + bottom_image);
}

void
Background::set_speed(float speed)
{
  SCRIPT_GUARD_VOID;
  object.set_speed(speed);
}

float
Background::get_color_red()
{
  SCRIPT_GUARD_RETURN(-1.f);
  return object.get_color().red;
}

float
Background::get_color_green()
{
  SCRIPT_GUARD_RETURN(-1.f);
  return object.get_color().green;
}

float
Background::get_color_blue()
{
  SCRIPT_GUARD_RETURN(-1.f);
  return object.get_color().blue;
}

float
Background::get_color_alpha()
{
  SCRIPT_GUARD_RETURN(-1.f);
  return object.get_color().alpha;
}

void
Background::set_color(float red, float green, float blue, float alpha)
{
  SCRIPT_GUARD_VOID;
  object.set_color(Color(red, green, blue, alpha));
}

void
Background::fade_color(float red, float green, float blue, float alpha, float time)
{
  SCRIPT_GUARD_VOID;
  object.fade_color(Color(red, green, blue, alpha), time);
}

void
Background::set_top_image_action(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_top_image_action(action);
}

void
Background::set_image_action(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_image_action(action);
}

void
Background::set_bottom_image_action(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_bottom_image_action(action);
}

void
Background::set_image_actions(const std::string& action)
{
  SCRIPT_GUARD_VOID;
  object.set_image_actions(action);
}

} // namespace scripting

/* EOF */
