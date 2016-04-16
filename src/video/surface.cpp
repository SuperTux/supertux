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
#include <SDL_ttf.h>

#include "video/texture.hpp"
#include "video/video_system.hpp"

SurfacePtr
Surface::create(const std::string& file)
{
  return SurfacePtr(new Surface(file));
}

SurfacePtr
Surface::create(const std::string& file, const Rect& rect)
{
  return SurfacePtr(new Surface(file, rect));
}

SurfacePtr
Surface::create(TexturePtr texture)
{
  return SurfacePtr(new Surface(texture));
}

Surface::Surface(const std::string& file) :
  texture(TextureManager::current()->get(file)),
  surface_data(),
  rect(0, 0,
      Size(texture->get_image_width(),
           texture->get_image_height())),
  flipx(false)
{
  surface_data = VideoSystem::current()->new_surface_data(*this);
}

Surface::Surface(TexturePtr texture_) :
  texture(texture_),
  surface_data(),
  rect(0, 0,
      Size(texture_->get_image_width(),
           texture_->get_image_height())),
  flipx(false)
{
  surface_data = VideoSystem::current()->new_surface_data(*this);
}

Surface::Surface(const std::string& file, const Rect& rect_) :
  texture(TextureManager::current()->get(file, rect_)),
  surface_data(),
  rect(0, 0, Size(rect_.get_width(), rect_.get_height())),
  flipx(false)
{
  surface_data = VideoSystem::current()->new_surface_data(*this);
}

Surface::Surface(const Surface& rhs) :
  texture(rhs.texture),
  surface_data(),
  rect(rhs.rect),
  flipx(false)
{
  surface_data = VideoSystem::current()->new_surface_data(*this);
}

Surface::~Surface()
{
  VideoSystem::current()->free_surface_data(surface_data);
}

SurfacePtr
Surface::clone() const
{
  SurfacePtr surface(new Surface(*this));
  return surface;
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

TexturePtr
Surface::get_texture() const
{
  return texture;
}

SurfaceData*
Surface::get_surface_data() const
{
  return surface_data;
}

int
Surface::get_x() const
{
  return rect.left;
}

int
Surface::get_y() const
{
  return rect.top;
}

int
Surface::get_width() const
{
  return rect.get_width();
}

int
Surface::get_height() const
{
  return rect.get_height();
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
