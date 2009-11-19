//  SuperTux
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

#include "video/surface.hpp"

#include <config.h>

#include <SDL.h>

#include "video/texture.hpp"
#include "video/video_systems.hpp"

Surface::Surface(const std::string& file) :
  texture(texture_manager->get(file)),
  surface_data(),
  x(0),
  y(0), 
  w(0), 
  h(0),
  flipx(false)
{
  texture->ref();
  w = texture->get_image_width();
  h = texture->get_image_height();
  surface_data = new_surface_data(*this);
}

Surface::Surface(const std::string& file, int x, int y, int w, int h) :
  texture(texture_manager->get(file)),
  surface_data(),
  x(x), 
  y(y),
  w(w),
  h(h),
  flipx(false)
{
  texture->ref();
  surface_data = new_surface_data(*this);
}

Surface::Surface(const Surface& other) :
  texture(other.texture),
  surface_data(),
  x(other.x), 
  y(other.y),
  w(other.w), 
  h(other.h),
  flipx(false)
{
  texture->ref();
  surface_data = new_surface_data(*this);
}

const Surface& 
Surface::operator=(const Surface& other)
{
  other.texture->ref();
  texture->unref();
  texture = other.texture;
  x = other.x;
  y = other.y;
  w = other.w;
  h = other.h;
  return *this;
}

Surface::~Surface()
{
  free_surface_data(surface_data);
  texture->unref();
}

/** flip the surface horizontally */
void Surface::hflip()
{
  flipx = !flipx;
}

bool Surface::get_flipx() const
{
  return flipx;
}

Texture* 
Surface::get_texture() const
{
  return texture;
}

void* 
Surface::get_surface_data() const
{
  return surface_data;
}

int
Surface::get_x() const
{
  return x;
}

int
Surface::get_y() const
{
  return y;
}

int 
Surface::get_width() const
{
  return w;
}

int 
Surface::get_height() const
{
  return h;
}

Vector
Surface::get_position() const
{
  return Vector(get_x(), get_y()); 
}

Vector
Surface::get_size() const
{
  return Vector(get_width(), get_height()); 
}

/* EOF */
