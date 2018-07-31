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

Surface::Surface(const std::string& file) :
  m_texture(TextureManager::current()->get(file)),
  m_rect(0, 0,
         Size(m_texture->get_image_width(),
              m_texture->get_image_height())),
  m_flipx(false)
{
}

Surface::Surface(const std::string& file, const Rect& rect_) :
  m_texture(TextureManager::current()->get(file, rect_)),
  m_rect(0, 0, Size(rect_.get_width(), rect_.get_height())),
  m_flipx(false)
{
}

Surface::Surface(const Surface& rhs) :
  m_texture(rhs.m_texture),
  m_rect(rhs.m_rect),
  m_flipx(false) // FIXME: Why no copy here?
{
}

Surface::~Surface()
{
}

SurfacePtr
Surface::clone() const
{
  SurfacePtr surface(new Surface(*this));
  return surface;
}

/** flip the surface horizontally */
void
Surface::hflip()
{
  m_flipx = !m_flipx;
}

bool
Surface::get_flipx() const
{
  return m_flipx;
}

TexturePtr
Surface::get_texture() const
{
  return m_texture;
}

int
Surface::get_x() const
{
  return m_rect.left;
}

int
Surface::get_y() const
{
  return m_rect.top;
}

int
Surface::get_width() const
{
  return m_rect.get_width();
}

int
Surface::get_height() const
{
  return m_rect.get_height();
}

Vector
Surface::get_position() const
{
  return Vector(static_cast<float>(get_x()),
                static_cast<float>(get_y()));
}

Vector
Surface::get_size() const
{
  return Vector(static_cast<float>(get_width()),
                static_cast<float>(get_height()));
}

/* EOF */
