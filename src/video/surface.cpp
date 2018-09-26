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

#include "util/reader_mapping.hpp"
#include "video/texture.hpp"
#include "video/texture_manager.hpp"
#include "video/video_system.hpp"

SurfacePtr
Surface::from_reader(const ReaderMapping& mapping)
{
  TexturePtr diffuse_texture;
  ReaderMapping diffuse_texture_mapping;
  if (mapping.get("diffuse-texture", diffuse_texture_mapping))
  {
    diffuse_texture = TextureManager::current()->get(diffuse_texture_mapping);
  }

  TexturePtr displacement_texture;
  ReaderMapping displacement_texture_mapping;
  if (mapping.get("displacement-texture", displacement_texture_mapping))
  {
    displacement_texture = TextureManager::current()->get(displacement_texture_mapping);
  }

  Vector translate;
  std::vector<float> translate_v;
  if (mapping.get("translate", translate_v))
  {
    translate.x = translate_v[0];
    translate.y = translate_v[1];
  }

  Vector scale;
  std::vector<float> scale_v;
  if (mapping.get("scale", scale_v))
  {
    scale.x = scale_v[0];
    scale.y = scale_v[1];
  }

  float rotate = 0.0f;
  mapping.get("rotate", rotate);

  Vector rotate_center;
  std::vector<float> rotate_center_v;
  if (mapping.get("rotate-center", rotate_center_v))
  {
    rotate_center.x = rotate_center_v[0];
    rotate_center.y = rotate_center_v[1];
  }

  DrawingEffect effect = NO_EFFECT;
  std::vector<bool> flip_v;
  if (mapping.get("flip", flip_v))
  {
    effect ^= flip_v[0] ? HORIZONTAL_FLIP : NO_EFFECT;
    effect ^= flip_v[1] ? VERTICAL_FLIP : NO_EFFECT;
  }

  return SurfacePtr(new Surface(diffuse_texture, displacement_texture,
                                translate,
                                scale,
                                rotate,
                                rotate_center,
                                effect));
}

SurfacePtr
Surface::from_file(const std::string& filename)
{
  TexturePtr texture = TextureManager::current()->get(filename);
  return SurfacePtr(new Surface(texture, TexturePtr(), Vector(), Vector(1.0f, 1.0f), 0.0f, Vector(), NO_EFFECT));
}

SurfacePtr
Surface::from_file(const std::string& filename, const Rect& rect)
{
  TexturePtr texture = TextureManager::current()->get(filename, rect);
  return SurfacePtr(new Surface(texture, TexturePtr(), Vector(), Vector(1.0f, 1.0f), 0.0f, Vector(), NO_EFFECT));
}

Surface::Surface(const TexturePtr& diffuse_texture,
                 const TexturePtr& displacement_texture,
                 const Vector& translate,
                 const Vector& scale,
                 float rotate,
                 const Vector& rotate_center,
                 DrawingEffect effect) :
  m_diffuse_texture(diffuse_texture),
  m_displacement_texture(displacement_texture),
  m_rect(0, 0, diffuse_texture->get_image_width(), diffuse_texture->get_image_height()),
  m_translate(translate),
  m_scale(scale),
  m_rotate(rotate),
  m_rotate_center(rotate_center),
  m_effect(effect)
{
}

Surface::Surface(const Surface& rhs) :
  m_diffuse_texture(rhs.m_diffuse_texture),
  m_displacement_texture(rhs.m_displacement_texture),
  m_rect(rhs.m_rect),
  m_translate(rhs.m_translate),
  m_scale(rhs.m_scale),
  m_rotate(rhs.m_rotate),
  m_rotate_center(rhs.m_rotate_center),
  m_effect(rhs.m_effect)
{
}

SurfacePtr
Surface::from_texture(const TexturePtr& texture)
{
  Rect srcrect(0, 0, texture->get_image_width(), texture->get_image_height());
  return SurfacePtr(new Surface(texture, TexturePtr(), Vector(), Vector(1.0f, 1.0f), 0.0f, Vector(), NO_EFFECT));
}

Surface::~Surface()
{
}

SurfacePtr
Surface::clone(DrawingEffect effect) const
{
  SurfacePtr surface(new Surface(*this));
  surface->m_effect ^= effect;
  return surface;
}

DrawingEffect
Surface::get_effect() const
{
  return m_effect;
}

TexturePtr
Surface::get_texture() const
{
  return m_diffuse_texture;
}

TexturePtr
Surface::get_displacement_texture() const
{
  return m_displacement_texture;
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
